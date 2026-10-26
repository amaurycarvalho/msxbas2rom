# Spec Delta

## MODIFIED Requirements

### Requirement: Terminal state is captured deterministically

The workflow SHALL capture the program state when execution reaches the program
end, and SHALL use a configurable emulated-time fallback for programs that do not
terminate. Terminal capture SHALL happen before the end routine restores text
mode or otherwise cleans up the display state.

#### Scenario: Terminating program captured at end

- **WHEN** a program reaches its implicit or explicit end
- **THEN** RAM and VRAM are captured at the pre-cleanup END entry
- **AND** the captured state still reflects the program's graphics mode

#### Scenario: Non-terminating program captured by fallback

- **WHEN** a program loops or waits for input without reaching its end
- **THEN** RAM and VRAM are captured after the configured emulated-time fallback

#### Scenario: Implicit end is captured before text restore

- **WHEN** a program falls off its last line and terminates implicitly
- **THEN** the capture breakpoint is the pre-cleanup END entry
- **AND** `END_PGM` is used only as a fallback when that entry is unavailable

#### Scenario: Captured mode conflicts with source mode

- **WHEN** the captured VDP registers indicate text mode but the program's last
  literal SCREEN is a graphics mode
- **THEN** the report records the conflict
- **AND** the workflow does not silently select text-mode VRAM regions
