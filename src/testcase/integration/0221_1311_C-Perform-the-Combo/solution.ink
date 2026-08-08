// Translated from solution.cpp.

var T: dynamic;

var n: dynamic;

var m: dynamic;

var s = cpp_array(200043);

var p = cpp_array(200043);

var cnt = cpp_array(200043);

var ans = cpp_array(43);

func main(argc: dynamic, argv: dynamic)
{
  scanf("%d", (&T));
  while (cpp_update(T, "--"))
  {
    scanf("%d%d", (&n), (&m));
    scanf("%s", s);
    {
      var i = 0;
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
    var del = 0;
    var sci = 0;
    {
      var i = 0;
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
      var i = 0;
      while ((i < n))
      {
        ans[(s[i] - cpp_char("a"))] += (cnt[i] + 1);
        i += 1;
      }
    }
    {
      var i = 0;
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
