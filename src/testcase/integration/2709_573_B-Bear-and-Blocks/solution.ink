// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var h: dynamic = cpp_array(100005);

var ans: dynamic = cpp_array(100005);

var res: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(n);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&h[i]));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      ans[i] = min((1 + ans[(i - 1)]), h[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = n;
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
