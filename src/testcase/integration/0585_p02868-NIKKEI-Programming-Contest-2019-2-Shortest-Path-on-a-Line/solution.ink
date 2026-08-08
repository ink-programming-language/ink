// Translated from solution.cpp.

var MAX = cpp_expression("#inclu");

var edge = cpp_array(MAX);

var val: dynamic;

func main(argument_0: dynamic)
{
  var N: dynamic;
  var M: dynamic;
  read(N, M);
  val[1] = 0;
  val[N] = ((1 * 998244353) * 998244353);
  {
    var i = 0;
    while ((i < M))
    {
      read(edge[i].first.first, edge[i].first.second, edge[i].second);
      i += 1;
    }
  }
  sort(edge, (edge + M));
  for (var e in edge)
  {
    if ((!e.second))
    {
      break;
    }
    var lr = val.lower_bound(e.first.first);
    var ee = val.lower_bound(e.first.second);
    if ((ee->second > (lr->second + e.second)))
    {
      val[e.first.second] = (lr->second + e.second);
      ee = val.upper_bound((e.first.second - 1));
      while (((ee->second >= (lr->second + e.second)) && (ee != lr)))
      {
        var eee = ee;
        ee -= 1;
        if ((eee->first != e.first.second))
        {
          val.erase(eee);
        }
      }
    }
  }
  if ((val[N] < ((1 * 998244353) * 998244353)))
  {
    write(val[N], "\n");
  } else
  {
    write(-1, "\n");
  }
  return 0;
}
