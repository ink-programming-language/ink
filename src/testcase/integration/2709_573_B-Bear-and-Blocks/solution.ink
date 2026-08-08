// Translated from solution.cpp.

var n: dynamic;

var h = cpp_array(100005);

var ans = cpp_array(100005);

var res: dynamic;

func main()
{
  read(n);
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&h[i]));
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      ans[i] = min((1 + ans[(i - 1)]), h[i]);
      i += 1;
    }
  }
  {
    var i = n;
    while ((i > 0))
    {
      ans[i] = min((1 + ans[(i + 1)]), ans[i]);
      res = max(res, ans[i]);
      i -= 1;
    }
  }
  write(res, "\n");
  return 0;
}
