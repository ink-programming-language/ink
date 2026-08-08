// Translated from solution.cpp.

var N = (1500 + 10);

var s = cpp_array(N);

var n: dynamic;

var q: dynamic;

var a = cpp_array(N);

var memo = cpp_array(30, N);

func solve(x: dynamic, c: dynamic)
{
  var j = 0;
  var miss = 0;
  var ret = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      if (((a[i] != c) && (j < i)))
      {
        miss += 1;
        j = i;
      }
      while ((((j + 1) <= n) && (((a[(j + 1)] == c) || ((miss + 1) <= x)))))
      {
        j += 1;
        if ((a[j] != c))
        {
          miss += 1;
        }
      }
      if ((miss <= x))
      {
        ret = max(ret, ((j - i) + 1));
      }
      if ((a[i] != c))
      {
        miss -= 1;
      }
      i += 1;
    }
  }
  return ret;
}

func main()
{
  scanf("%d%s%d", (&n), s, (&q));
  {
    var i = 0;
    while ((i < n))
    {
      a[(i + 1)] = cpp_cast(((s[i] - cpp_char("a"))));
      i += 1;
    }
  }
  memset(memo, -1, cpp_sizeof(memo));
  while (cpp_update(q, "--"))
  {
    var x: dynamic;
    var c: dynamic;
    var ch = cpp_array(2);
    scanf("%d %s", (&x), ch);
    c = cpp_cast(((ch[0] - cpp_char("a"))));
    if ((memo[x][c] == -1))
    {
      memo[x][c] = solve(x, c);
    }
    printf("%d\n", memo[x][c]);
  }
  return 0;
}
