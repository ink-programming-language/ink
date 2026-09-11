// Translated from solution.cpp.

var T: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var s: dynamic = cpp_array(200043);

var p: dynamic = cpp_array(200043);

var cnt: dynamic = cpp_array(200043);

var ans: dynamic = cpp_array(43);

func main(argc: dynamic, argv: dynamic) -> dynamic
{
  scanf("%d", (&T));
  while (cpp_update(T, "--"))
  {
    scanf("%d%d", (&n), (&m));
    scanf("%s", s);
    {
      var i: dynamic = 0;
      while ((i < m))
      {
        scanf("%d", (&p[i]));
        p[i] -= 1;
        i += 1;
      }
    }
    sort(p, (p + m));
    memset(cnt, 0, cpp_sizeof((cnt)));
    memset(ans, 0, cpp_sizeof((ans)));
    var del: dynamic = 0;
    var sci: dynamic = 0;
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        cnt[i] += (cpp_cast(m) - del);
        while (((i == p[sci]) && (sci < m)))
        {
          sci += 1;
          del += 1;
        }
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        ans[(s[i] - cpp_char("a"))] += (cnt[i] + 1);
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < 26))
      {
        printf("%d ", ans[i]);
        i += 1;
      }
    }
    printf("\n");
  }
  return 0;
}
