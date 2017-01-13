::clang++ -shared -o test_output.dll coverage_main.cpp -I.. -DUNICODE -fsanitize-coverage=trace-cmp
::clang++ -shared -S -o test_output.s coverage_main.cpp -I.. -DUNICODE -fsanitize-coverage=trace-pc
:: -fsanitize-coverage=trace-cmp
:: clang 4.0 only -fsanitize-coverage=trace-div -fsanitize-coverage=trace-gep

"C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\ClangC2\bin\x86\clang" -shared -fexceptions -o test_output.dll coverage_main.cpp -I.. -DUNICODE -fsanitize-coverage=trace-pc
::"C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\ClangC2\bin\x86\clang" -S -o test_output.s coverage_main.cpp -I.. -DUNICODE -fsanitize-coverage=trace-pc
