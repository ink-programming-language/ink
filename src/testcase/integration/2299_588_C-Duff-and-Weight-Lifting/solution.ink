// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var num: dynamic;
  var x: dynamic;
  var y: dynamic;
  var ans: dynamic;
  while ((~scanf("%d", (&n))))
  {
    var q: dynamic;
    ans = 0;
    {
      var i = 0;
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
