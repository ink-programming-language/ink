// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var num: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var ans: dynamic = cpp_uninitialized();
  while ((~scanf("%d", (&n))))
  {
    var q: dynamic = cpp_uninitialized();
    ans = 0;
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        scanf("%d", (&num));
        q.push(num);
        i += 1;
      }
    }
    {
      while (true)
      {
        if ((q.size() == 1))
        {
          ans += 1;
          break;
        }
        x = q.top();
        q.pop();
        y = q.top();
        if ((x != y))
        {
          ans += 1;
        }
        if ((x == y))
        {
          q.pop();
          q.push(cpp_update(x, "++"));
        }
      }
    }
    printf("%d\n", ans);
  }
}
