// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int (i)=0;(i)<(int)(n);++(i))");
}

func rer(i: dynamic, l: dynamic, u: dynamic)
{
  cpp_macro("for(int (i)=(int)(l);(i)<=(int)(u);++(i))");
}

func reu(i: dynamic, l: dynamic, u: dynamic)
{
  cpp_macro("for(int (i)=(int)(l);(i)<(int)(u);++(i))");
}

var INF = 0x3f3f3f3f;

var INFL = 0x3f3f3f3f3f3f3f3f;

func amin(x: dynamic, y: dynamic)
{
  if ((y < x))
  {
    x = y;
  }
}

func amax(x: dynamic, y: dynamic)
{
  if ((x < y))
  {
    x = y;
  }
}

func main()
{
  var n: dynamic;
  while ((~scanf("%d", (&n))))
  {
    var K = 3;
    var v = cpp_array(K);
    rep(k, 3);
    sort(v[k].begin(), v[k].end());
    var ans = 0;
    rer(first, 1, 2);
    if ((!v[first].empty()))
    {
      var rem = cpp_array(3);
      rer(k, 1, 2)[k] = v[k];
      var sum = 1;
      rem[first].pop_back();
      sum += v[0].size();
      var r = first;
      while ((!rem[r].empty()))
      {
        sum += 1;
        rem[r].pop_back();
        (cpp_assign(r, "+=", r)) %= K;
      }
      if ((!rem[(3 - r)].empty()))
      {
        sum += 1;
      }
      amax(ans, sum);
    }
    if ((!v[0].empty()))
    {
      amax(ans, 1);
    }
    printf("%d\n", ans);
  }
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
      var a: dynamic;
      scanf("%d", (&a));
      v[(a % 3)].push_back(a);
    }
