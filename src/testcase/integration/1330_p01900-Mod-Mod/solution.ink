// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int (i)=0;(i)<(int)(n);++(i))");
}

func rer(i: dynamic, l: dynamic, u: dynamic) -> dynamic
{
  cpp_macro("for(int (i)=(int)(l);(i)<=(int)(u);++(i))");
}

func reu(i: dynamic, l: dynamic, u: dynamic) -> dynamic
{
  cpp_macro("for(int (i)=(int)(l);(i)<(int)(u);++(i))");
}

var INF: dynamic = 0x3f3f3f3f;

var INFL: dynamic = 0x3f3f3f3f3f3f3f3f;

func amin(x: dynamic, y: dynamic) -> dynamic
{
  if ((y < x))
  {
    x = y;
  }
}

func amax(x: dynamic, y: dynamic) -> dynamic
{
  if ((x < y))
  {
    x = y;
  }
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  while ((~scanf("%d", (&n))))
  {
    var K: dynamic = 3;
    var v: dynamic = cpp_array(K);
    rep(k, 3);
    sort(v[k].begin(), v[k].end());
    var ans: dynamic = 0;
    rer(first, 1, 2);
    if ((!v[first].empty()))
    {
      var rem: dynamic = cpp_array(3);
      rer(k, 1, 2)[k] = v[k];
      var sum: dynamic = 1;
      rem[first].pop_back();
      sum += v[0].size();
      var r: dynamic = first;
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

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      var a: dynamic = cpp_uninitialized();
      scanf("%d", (&a));
      v[(a % 3)].push_back(a);
    }
