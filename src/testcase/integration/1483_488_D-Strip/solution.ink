// Translated from solution.cpp.

var maxn = 100010;

var INF = (1e9 + 7);

var n: dynamic;

var s: dynamic;

var l: dynamic;

var dp = cpp_array(maxn);

var a = cpp_array(maxn);

var st: dynamic;

var v: dynamic;

func main()
{
  read(n, s, l);
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  {
    var i = 1;
    var j = 1;
    while ((i <= n))
    {
      st.insert(a[i]);
      while ((((*st.rbegin()) - (*st.begin())) > s))
      {
        st.erase(st.find(a[j]));
        if (((i - j) >= l))
        {
          v.erase(v.find(dp[(j - 1)]));
        }
        j += 1;
      }
      if ((((i - j) + 1) >= l))
      {
        v.insert(dp[(i - l)]);
      }
      if ((v.begin() == v.end()))
      {
        dp[i] = INF;
      } else
      {
        dp[i] = ((*v.begin()) + 1);
      }
      i += 1;
    }
  }
  if ((dp[n] >= INF))
  {
    write(-1, "\n");
  } else
  {
    write(dp[n], "\n");
  }
  return 0;
}
