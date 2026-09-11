// Translated from solution.cpp.

var INF: dynamic = (~((1 << 31)));

var eps: dynamic = 1e-6;

var PI: dynamic = 3.1415926535;

var MOD: dynamic = (1e9 + 7);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var s: dynamic = cpp_uninitialized();
  read(s);
  var st: dynamic = cpp_uninitialized();
  {
    int_cpp(i) = 0;
    while (((i) < (s.size())))
    {
      var g: dynamic = (s[i] == cpp_char("+"));
      if ((st.size() && (st.top() == g)))
      {
        st.pop();
      } else
      {
        st.push(g);
      }
      (i) += 1;
    }
  }
  if ((st.size() == 0))
  {
    puts("Yes");
  } else
  {
    puts("No");
  }
  return 0;
}
