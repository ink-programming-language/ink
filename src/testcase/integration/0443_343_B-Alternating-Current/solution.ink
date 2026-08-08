// Translated from solution.cpp.

var INF = (~((1 << 31)));

var eps = 1e-6;

var PI = 3.1415926535;

var MOD = (1e9 + 7);

var n: dynamic;

var m: dynamic;

func main()
{
  var s: dynamic;
  read(s);
  var st: dynamic;
  {
    int_cpp(i) = 0;
    while (((i) < (s.size())))
    {
      var g = (s[i] == cpp_char("+"));
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
