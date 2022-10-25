\page ckpt_learn_ex_tofile_nonintrusive Non-Intrusive Program Example Serialize To File

\attention All Non-Intrusive serialize methods <b>MUST</b> be placed in the namespace of type which they serialize.

One potential application for this approach is to generate a restart file
for a simulation.

The full code for this example can be found here:
`examples/checkpoint_example_to_file_nonintrusive.cc`

\subsubsection ex Example source code:
\snippet examples/checkpoint_example_to_file_nonintrusive.cc Non-Intrusive Serialize structure to file
