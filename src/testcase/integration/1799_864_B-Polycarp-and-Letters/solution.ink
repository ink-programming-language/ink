// Translated from solution.cpp.

var N = (1e5 + 5);

var arr = cpp_array(N);

var A = cpp_array(N);

var ARR = cpp_array(N);

var visited = [];

var cnt = [];

var f: dynamic;

func main()
{
  var n: dynamic;
  var ans = 0;
  var s: dynamic;
  var st: dynamic;
  read(n, s);
  {
    var i = 0;
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
