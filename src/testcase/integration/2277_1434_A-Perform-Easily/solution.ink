// Translated from solution.cpp.

var L = 6;

var N = (1e5 + 5);

var A = cpp_array((L + 1));

var licz = cpp_array(N);

func main()
{
  {
    var i = 1;
    while ((i <= L))
    {
      scanf("%d", (&A[i]));
      i += 1;
    }
  }
  var n: dynamic;
  scanf("%d", (&n));
  var broom: dynamic;
  {
    var i = 1;
    while ((i <= n))
    {
      var x: dynamic;
      scanf("%d", (&x));
      {
        var j = 1;
        while ((j <= L))
        {
          if ((x > A[j]))
          {
            broom.push_back([(x - A[j]), i]);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  sort(broom.begin(), broom.end());
  var not_covered = n;
  var i = 0;
  var j = -1;
  while ((not_covered > 0))
  {
    j += 1;
    if ((licz[broom[j].second] == 0))
    {
      not_covered -= 1;
    }
    licz[broom[j].second] += 1;
  }
  var res = (broom[j].first - broom[i].first);
  while ((i < int_cpp(broom.size())))
  {
    licz[broom[i].second] -= 1;
    while (((j < (int_cpp(broom.size()) - 1)) && (licz[broom[i].second] == 0)))
    {
      j += 1;
      licz[broom[j].second] += 1;
    }
    if ((licz[broom[i].second] == 0))
    {
      break;
    }
    i += 1;
    res = min(res, (broom[j].first - broom[i].first));
  }
  printf("%d\n", res);
}
