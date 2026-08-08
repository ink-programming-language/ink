// Translated from solution.cpp.

func FOR(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0; i<int(n); i++)");
}

func FOR1(i: dynamic, m: dynamic, n: dynamic)
{
  cpp_macro("for(int i=int(m); i<int(n); i++)");
}

var N: dynamic;

var L: dynamic;

var A: dynamic;

func main(argc: dynamic, argv: dynamic)
{
  var t: dynamic;
  var v: dynamic;
  var total_heat: dynamic;
  scanf("%lld%lld", (&N), (&L));
  scanf("%lld%lld", (&t), (&v));
  A.push_front(make_pair(v, t));
  total_heat = (cpp_cast(v) * t);
  printf("%.7f\n", (cpp_cast(total_heat) / L));
  FOR1(i, 1, N);
  {
    var back: dynamic;
    var vol: dynamic;
    scanf("%lld%lld", (&t), (&v));
    total_heat += (cpp_cast(v) * t);
    vol = v;
    while ((vol > 0))
    {
      back = A.back();
      A.pop_back();
      vol -= back.first;
      total_heat -= (back.first * back.second);
    }
    if ((vol != 0))
    {
      A.push_back(make_pair((-vol), back.second));
      total_heat += ((-vol) * back.second);
    }
    var heat = (v * t);
    vol = v;
    while (((A.size() > 0) && ((heat / vol) <= A.front().second)))
    {
      heat += (A.front().first * A.front().second);
      vol += A.front().first;
      A.pop_front();
    }
    A.push_front(make_pair(vol, (heat / vol)));
    printf("%.7f\n", (cpp_cast(total_heat) / L));
  }
  return 0;
}
