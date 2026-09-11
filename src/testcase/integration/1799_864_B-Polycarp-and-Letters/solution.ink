// Translated from solution.cpp.

var N: dynamic = (1e5 + 5);

var arr: dynamic = cpp_array(N);

var A: dynamic = cpp_array(N);

var ARR: dynamic = cpp_array(N);

var visited: dynamic = [];

var cnt: dynamic = [];

var f: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var ans: dynamic = 0;
  var s: dynamic = cpp_uninitialized();
  var st: dynamic = cpp_uninitialized();
  read(n, s);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((s[i] < cpp_char("a")))
      {
        ans = max(ans, cpp_cast(st.size()));
        st.clear();
      } else
      {
        st.insert(s[i]);
      }
      i += 1;
    }
  }
  write(max(ans, cpp_cast(st.size())));
  return 0;
}
