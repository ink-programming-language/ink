// Translated from solution.cpp.

var p: dynamic = cpp_array(400100);

var s: dynamic = cpp_array(400100);

var cnt: dynamic = cpp_array(1000100);

var ct: dynamic = cpp_array(400100);

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    scanf("%d", (&n));
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        scanf("%d", (&p[i]));
        i += 1;
      }
    }
    var len: dynamic = 0;
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        if (cpp_binary((cnt[p[i]] == 0), "and", (i > 1)))
        {
          ct[cpp_update(len, "++")] = cnt[p[(i - 1)]];
          cnt[p[i]] += 1;
          i += 1;
          continue;
        }
        cnt[p[i]] += 1;
        i += 1;
      }
    }
    ct[cpp_update(len, "++")] = cnt[p[n]];
    {
      var i: dynamic = 1;
      while ((i <= len))
      {
        s[i] = (s[(i - 1)] + ct[i]);
        i += 1;
      }
    }
    var id: dynamic = ((upper_bound((s + 1), ((s + len) + 1), (n / 2)) - s) - 1);
    var g: dynamic = ct[1];
    var st: dynamic = (upper_bound((s + 1), ((s + len) + 1), (2 * g)) - s);
    var ed: dynamic = (upper_bound((s + 1), ((s + len) + 1), (s[st] + g)) - s);
    if ((ed <= id))
    {
      printf("%d %d %d\n", g, (s[st] - g), (s[id] - s[st]));
    } else
    {
      puts("0 0 0");
    }
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        cnt[p[i]] = 0;
        i += 1;
      }
    }
    {
      var i: dynamic = 1;
      while ((i <= len))
      {
        s[i] = 0;
        ct[i] = 0;
        i += 1;
      }
    }
  }
  return 0;
}
