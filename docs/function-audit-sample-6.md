# Sixth random function-audit sample

Reviewed on 23 October 2026 against live Ghidra program `SLES_514.73`, image base
`00100000`. Five candidates were drawn without replacement using PowerShell
`Get-Random -Count 5` from the version 5 queue of 2,148 candidates. Previously
sampled functions were excluded; there were no redraws after inspection.

The draw is saved in `out/function-audit-filtered-v5/random-sample-6.json`.
`random-sample-6-evidence.json` in that directory contains live decompilations,
disassembly and supporting layouts. All five current source definitions match
their packet source after line-ending normalization; all five live PS2
decompilations match the saved snapshot. These generated artifacts are local.

| Function | PS2 address | Assessment |
|---|---|---|
| `CActorMovingPlatform::BehaviourTrajectory_Manage` | `00155ae0` | Extracted switch/sound helper and typed stream layout |
| `CAddOnGenerator::Term` | `00397440` | Allocator naming and explicit Windows lifecycle bookkeeping |
| `CActorMicken::StateMickenKickImpactFly` | `0014d9f0` | Extracted animation-end predicate |
| `CActorMoney::SaveContext` | `003634d0` | Native virtual dispatch preserves the original call |
| `CActorBlazer::Blazer_UpdatePosition` | `003d83d0` | **Confirmed missing quaternion restoration** |

One source defect was confirmed against PS2 instructions. The other four have
explained differences with no additional defect established in this review.
No game code, filter rules or review exceptions were changed. No runtime
reproduction was attempted, and this sample does not estimate the queue's defect rate.

## Blazer: saved rotation was replaced by a self-assignment

In `src/b-witch/ActorBlazer.cpp:539`, the port calls the tilt and position helpers
and then executes `this->rotationQuat = this->rotationQuat`. It never saves the
pre-call value. The unused original temporary declarations remain in the function.

The original saves all 16 bytes of `rotationQuat` before the tilt call:

- `003d8460` loads actor offset `0x40` with `lq`; `003d8470` stores the value at
  stack offset `0x60`.
- After all movement branches, `003d8580`–`003d859c` load the four saved words
  and store them back to actor offsets `0x40`, `0x44`, `0x48` and `0x4c`.
- Restoration precedes the optional Euler-angle update at `003d85b0`, target
  orientation update at `003d85d8`, and final position update at `003d8610`.

This restoration has a concrete purpose. The tilt-active branch calls
`CActor::SV_UpdateMatrix_Rel` (`00118380`) at `003d8504` with its rotation-update
argument set to `1`. That callee computes Euler angles and calls
`SetVectorFromAngles` at `00118450`, passing actor offset `0x40` as the output
(`00118438`). The port's counterpart at `Actor.cpp:5029` also rewrites the
quaternion. The original restores the prior heading after this operation; the
port retains the helper's result. Later orientation logic can therefore receive
a different heading. A visible gameplay symptom has not been reproduced.

The audit only flags a missing literal `32`: Ghidra represents the saved x/y
pair as an eight-byte temporary with a right shift by `0x20`. Disassembly shows
a bit-preserving save/restore, not numerical float conversions. Blanket filtering
of packing shifts or temporary expressions would hide the only current signal
for this actual omission.

## Moving platform: switch and sound loops moved into a helper

`ActorMovingPlatform.cpp:1459` calls `TriggerSwitches(3)` when entering the
waiting state and `TriggerSwitches(0)` when leaving transition B for movement.
The helper at line 2532 contains the original loops: select entries by condition,
switch the target, post the switch, switch on the camera event, and start the
entry's sound in the first available permitted instance slot.

The condition-0 loop calls `Switch`, `PostSwitch` and `SwitchOn` at
`00155c08`, `00155c14` and `00155c20`; the condition-3 loop repeats them at
`00155d6c`, `00155d78` and `00155d84`. The reserved sound-slot count is the sum
of flag bits 0 and 1. The original checks availability and starts sound at
`00155c74`/`00155c9c` and `00155ddc`/`00155e04`, matching the helper.

Ghidra and the source agree on the entry's `0x48` stride, target offset `8`,
camera-event offset `0x24` and sound-reference offset `0x44`. The stream starts
with a four-byte count. These layouts explain the raw offset/stride literals.
No omitted switch or sound operation was found. The extracted helper includes
branches and a loop, so it falls outside the current transparent-helper rules.
This is a manual explanation, not a proof of all callback/timing behavior.

## Add-on termination: allocator spelling and Windows bookkeeping

`ActorBonusServices.cpp:679` frees `aInstances`, subtracts the generator's money
and bonus counts, then resets the shared state when all totals reach zero.
The original does those operations in the same order: free at `0039745c`,
counter updates at `00397468`–`003974c4`, and resets at `003974dc`–`00397540`.

The original `Free` at `001002b0` obtains the allocation header and invokes its
memory-handler free function. The port's `edMemFree` at `edMem.cpp:249` retains
that logic with logging and allocation tracking. Creation allocates the array
through `edMemAlloc` at `ActorBonusServices.cpp:605`.

The `PLATFORM_WIN` block additionally removes this generator from
`_gAddOnGeneratorInstances`, increments `_gAddOn_NbRemainingMinusTermed`, and
sets `_gDoneTerm`. Creation registers generators and uses `_gDoneTerm` to check
that the previous generator list is empty before creating a new set. The extra
counter has no reader in the current source. These are explicit port additions;
the lifecycle assertion can affect behavior, so full equivalence is not claimed.
No further defect was established from this function's findings.

## Micken: animation-end helper preserves all three tests

`ActorMicken.cpp:741` replaces the original inline animation test with
`CAnimation::IsCurrentLayerAnimEndReached(0)` (`Animation.cpp:3582`). That helper
checks the layer descriptor's animation type against the controller's current
type, then calls `edAnmBinMetaAnimator::IsLayerAnimEndReached`
(`Animation.cpp:2472`). The latter requires nonzero playback state and flag bit
`2` in `field_0xcc`; `STATE_ANIM_NONE` is zero.

The original performs exactly these tests at `0014daf8`–`0014db40`. The surrounding
ground flag, absolute vertical-speed threshold, horizontal-speed threshold,
state changes and behavior selection also agree. The extra literal `2` in the
PS2 packet belongs to the helper's animation-end flag. No missing condition
was found. Conditional, value-returning helpers are not currently expanded.

## Money save: virtual dispatch and arguments are preserved

`ActorMoney.cpp:83` obtains the active behavior, casts it to
`CBehaviourMoneyFlock*` and calls its virtual `SaveContext` method. The original
calls `GetBehaviour` at `003634f0`, loads vtable slot `0x4c` at `00363508`, and
performs the indirect call at `0036350c`, forwarding the returned behavior as
receiver and the original data, mode and size arguments.

Ghidra's `CBehaviourMoneyVTable` explicitly identifies slot `0x4c` as
`SaveContext`. The source declares that method virtual in `ActorMoney.h:61`;
`CBehaviourMoneyAddOn` overrides it at line 93. The cast does not suppress virtual
dispatch, so both flock and add-on behavior implementations remain reachable.
The findings are the named-call versus indirect-call representation, not an
omitted call or a forced choice of implementation.

## Implications for filtering

These examples suggest resolving virtual slots from reliable type information
and expanding a broader class of pure predicates or structured helpers. Both
need argument, control-flow and mutation checks. Allocator aliases need an
explicit verified mapping; Windows additions should remain visible as port
differences rather than be discarded wholesale. The Blazer finding is a concrete
reason to retain unexplained packing/restore signals. No new suppression was
added on the basis of these five functions.
