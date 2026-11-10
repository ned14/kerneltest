# Open work register — kerneltest

Tracking file, not an essay. Kerneltest previously had no open-work register
(this is the first one); entries here are P0/P1 findings from review, with the
commit that closed them. Only findings actually fixed are listed as closed —
nothing here is a "should work".

## Closed

All on branch `llm-fixes`, based on `5fedfbf`.

| Row | P0 | Fix commit | What was wrong |
| --- | --- | --- | --- |
| A5-1, G-9/A5-01/A5-2 | yes | `e9bb976` | `test/gen_sanitiser_coverage.sh` had no shebang and no error handling: a failed (or absent) compile still ran `objdump` and the script exited 0, producing a fabricated coverage report from a stale binary. Now `set -euo pipefail`, compiler via `CXX` (default `clang++-4.0`). |
| A3-2 | yes | `86e3f90` | `detail::hooks_container` had no empty specialisation, so a permuter with zero hooks — the default use — was an incomplete type and did not compile. |
| A3-1 | yes | `51aef3b` | `st_permute_parameters()` declared `parameter_permuter<true, Sequence>` while returning `<false, Sequence, Hooks...>`, and `mt_permute_parameters()` dropped the hooks from its declared type. Neither could be instantiated. |
| A4-1 | yes | `f124c80` | `copy_level()` tested `is_symlink(it->status())`, which follows the link, so a symlink to a directory was recursed into: out-of-tree content was copied into the workspace and a symlink loop aborted the run with ELOOP. Now `symlink_status()`. |
| F4 | yes | `125521e` | `collect_directories()` stored only "is a symlink" (a bool), so directory symlinks with different targets compared equal; and the file symlink check only ran when the *before* side was a link, so a model-only symlink was compared as a regular file and matched on size. Now the target is recorded, and symlink-ness is compared both ways. |

Verification for all five was done with standalone harnesses under
`/workspace/scratch/kerneltest-p0` compiled against `include/` (g++ 14.2 and
clang++ 19) — kerneltest's own suite needs Boost and cannot be built in this
container. Each commit body says exactly what was run and what was not.

## Open

- A5/F4 (Windows): the reparse-point block in `compare_directories()` still
  fetches both sides' reparse data but the enclosing test is before-side only,
  the mirror of the POSIX half of F4. Untouched: no Windows available to test.
- Nothing else has been recorded yet. Add entries as review finds them.
