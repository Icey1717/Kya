# RenderDoc Python Capture Analysis

## Local Setup

This repo has been tested with a locally built RenderDoc tree at:

```text
G:\repos\renderdoc
```

The standalone Python module is expected at:

```text
G:\repos\renderdoc\x64\Release\pymodules\renderdoc.pyd
```

The module was built from:

```text
G:\repos\renderdoc\qrenderdoc\Code\pyrenderdoc\pyrenderdoc_module.vcxproj
```

If it is missing, build it with MSBuild and override the toolset if needed:

```powershell
& 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\amd64\MSBuild.exe' `
  'G:\repos\renderdoc\qrenderdoc\Code\pyrenderdoc\pyrenderdoc_module.vcxproj' `
  /p:Configuration=Release `
  /p:Platform=x64 `
  /p:SolutionDir='G:\repos\renderdoc\' `
  /p:BuildProjectReferences=false `
  /p:PlatformToolset=v143 `
  /m
```

RenderDoc's Windows Python module is built for Python 3.6 in this checkout, so
use Python 3.6 rather than the system Python 3.14:

```powershell
py install -y 3.6
```

Before importing `renderdoc`, expose the RenderDoc DLL and Python module paths:

```powershell
$env:PATH = 'G:\repos\renderdoc\x64\Release;G:\repos\renderdoc\x64\Release\pymodules;' + $env:PATH
$env:PYTHONPATH = 'G:\repos\renderdoc\x64\Release\pymodules'
py -3.6 -c "import renderdoc as rd; print(rd.GetVersionString())"
```

Expected output for the tested build:

```text
1.45
```

## Opening A Capture

Minimal replay script:

```python
import renderdoc as rd

capture = r'C:\Users\icey1\Documents\renderdoc\AirPostBaseLine.rdc'

rd.InitialiseReplay(rd.GlobalEnvironment(), [])
cap = rd.OpenCaptureFile()

result = cap.OpenFile(capture, '', None)
if result != rd.ResultCode.Succeeded:
    raise RuntimeError('OpenFile failed: {}'.format(result))

if not cap.LocalReplaySupport():
    raise RuntimeError('Capture cannot be replayed locally')

result, controller = cap.OpenCapture(rd.ReplayOptions(), None)
if result != rd.ResultCode.Succeeded:
    raise RuntimeError('OpenCapture failed: {}'.format(result))

print('root actions:', len(controller.GetRootActions()))
print('textures:', len(controller.GetTextures()))
print('resources:', len(controller.GetResources()))

controller.Shutdown()
cap.Shutdown()
rd.ShutdownReplay()
```

Run it through Python 3.6 with the environment variables above.

## Useful API Calls

Action tree:

```python
controller.GetRootActions()
action.children
action.eventId
action.flags
action.numIndices
action.numInstances
action.GetName(controller.GetStructuredFile())
```

Pipeline state at an event:

```python
controller.SetFrameEvent(action.eventId, True)
pipe = controller.GetPipelineState()
pipe.GetGraphicsPipelineObject()
pipe.GetOutputTargets()
pipe.GetDepthTarget()
pipe.GetColorBlends()
pipe.GetDepthTestState()
pipe.GetRasterState()
pipe.GetVBuffers()
pipe.GetReadOnlyResources(rd.ShaderStage.Pixel)
```

Resources:

```python
controller.GetTextures()
controller.GetResources()
```

Counters:

```python
counters = controller.EnumerateCounters()
for counter in counters:
    desc = controller.DescribeCounter(counter)
    print(counter, desc.name, desc.unit)

results = controller.FetchCounters([rd.GPUCounter.EventGPUDuration])
```

`FetchCounters()` returns one result per event/counter. For generic GPU duration,
`CounterDescription.unit` is usually seconds. Convert to microseconds with
`seconds * 1_000_000`.

## Counter Workflow

1. Open the capture through the Python API.
2. Flatten `controller.GetRootActions()` into a list of actions.
3. Build an `eventId -> action` map for draw calls.
4. Call `controller.EnumerateCounters()` and verify
   `rd.GPUCounter.EventGPUDuration` is available.
5. Call `controller.FetchCounters([rd.GPUCounter.EventGPUDuration])`.
6. Join counter results back to action names by `eventId`.
7. Summarize total GPU duration by render pass/debug marker, and list the slowest
   draw events.
8. Keep structural draw counts separate from timed draw counts. Some small events
   can report `0` duration at counter precision, so use total draw counts for
   granularity context and nonzero timed draws for GPU cost attribution.


For renderer optimization work, include both `Timed draws` and `Total draws` in
generated summaries. `Timed draws` should mean events with nonzero counter
duration; `Total draws` should come from the flattened action tree. Use the
latter when validating draw granularity and the former when interpreting GPU
counter totals.

## GUI Equivalent

In RenderDoc's GUI:

- Use the Event Browser timer button to add a GPU duration column.
- Use Performance Counter Viewer -> Capture Counters -> Sample Counters for more
  counters.
- Save counter results as CSV from the Performance Counter Viewer.

## Notes

RenderDoc counter availability depends on GPU, driver, API, and installed vendor
counter plugins. Generic counters such as GPU duration are usually available;
AMD/NVIDIA/Intel hardware counters may require vendor-specific support.
