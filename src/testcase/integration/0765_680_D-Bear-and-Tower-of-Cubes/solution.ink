// Translated from solution.cpp.

var res: dynamic;

func my_pow(x: dynamic)
{
  return ((x * x) * x);
}

func dfs(m: dynamic, s: dynamic, v: dynamic)
{
  var x = 0;
  if ((m <= 0))
  {
    res = max(res, make_pair(s, v));
    return;
  }
  while ((my_pow((x + 1)) <= m))
  {
    x += 1;
  }
  dfs((m - my_pow(x)), (s + 1), (v + my_pow(x)));
  if (((x - 1) > 0))
  {
    dfs(((my_pow(x) - 1) - my_pow((x - 1))), (s + 1), (v + my_pow((x - 1))));
  }
}

func main()
{
  var m: dynamic;
  var i: dynamic;
  var t: dynamic;
  var j: dynamic;
  var N = 0;
  var X = 0;
  var ansn = 0;
  var ansX = 0;
  var flag = 0;
  var k: dynamic;
  var l: dynamic;
  scanf("%I64d", (&m));
  dfs(m, 0, 0);
  printf("%I64d %I64d\n", res.first, res.second);
}
