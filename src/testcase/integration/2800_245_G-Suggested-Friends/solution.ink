// Translated from solution.cpp.

var MAXN: dynamic = (10000 + 10);

var name: dynamic = cpp_uninitialized();

var g: dynamic = cpp_array(MAXN);

var mm: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var f: dynamic = cpp_array(MAXN);

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  read(m);
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var s1: dynamic = cpp_uninitialized();
      var s2: dynamic = cpp_uninitialized();
      read(s1, s2);
      if ((name.find(s1) == name.end()))
      {
        name.insert(s1);
        n += 1;
        mm[s1] = n;
      }
      if ((name.find(s2) == name.end()))
      {
        name.insert(s2);
        n += 1;
        mm[s2] = n;
      }
      var u: dynamic = mm[s1];
      var v: dynamic = mm[s2];
      g[u].insert(v);
      g[v].insert(u);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var cnt: dynamic = 0;
      var ans: dynamic = 0;
      {
        var j: dynamic = 1;
        while ((j <= n))
        {
          if (((i != j) && (g[i].find(j) == g[i].end())))
          {
            var temp: dynamic = 0;
            {
              var it: dynamic = g[i].begin();
              while ((it != g[i].end()))
              {
                if ((g[j].find((*it)) != g[j].end()))
                {
                  temp += 1;
                }
                it += 1;
              }
            }
            if ((temp == cnt))
            {
              ans += 1;
            }
            if ((temp > cnt))
            {
              cnt = temp;
              ans = 1;
            }
          }
          j += 1;
        }
      }
      f[i] = ans;
      i += 1;
    }
  }
  write(n, "\n");
  {
    var it: dynamic = name.begin();
    while ((it != name.end()))
    {
      write((*it), " ", f[mm[(*it)]], "\n");
      it += 1;
    }
  }
  return 0;
}
