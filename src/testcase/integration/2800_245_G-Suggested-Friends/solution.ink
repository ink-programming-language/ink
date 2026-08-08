// Translated from solution.cpp.

var MAXN = (10000 + 10);

var name: dynamic;

var g = cpp_array(MAXN);

var mm: dynamic;

var m: dynamic;

var n: dynamic;

var f = cpp_array(MAXN);

func main()
{
  ios.sync_with_stdio(0);
  read(m);
  {
    var i = 0;
    while ((i < m))
    {
      var s1: dynamic;
      var s2: dynamic;
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
      var u = mm[s1];
      var v = mm[s2];
      g[u].insert(v);
      g[v].insert(u);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      var cnt = 0;
      var ans = 0;
      {
        var j = 1;
        while ((j <= n))
        {
          if (((i != j) && (g[i].find(j) == g[i].end())))
          {
            var temp = 0;
            {
              var it = g[i].begin();
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
    var it = name.begin();
    while ((it != name.end()))
    {
      write((*it), " ", f[mm[(*it)]], "\n");
      it += 1;
    }
  }
  return 0;
}
