// Translated from solution.cpp.

var n: dynamic;

var l: dynamic;

var v: dynamic;

func main()
{
  scanf("%d %d", (&n), (&l));
  {
    var i = 0;
    while ((i < n))
    {
      var x: dynamic;
      scanf("%d", (&x));
      if ((x >= l))
      {
        v.push_back(x);
      }
      i += 1;
    }
  }
  sort(v.begin(), v.end());
  var ms = 0;
  {
    var i = 0;
    while ((i < v.size()))
    {
      var count = 0;
      {
        var j = i;
        while ((j < v.size()))
        {
          count += (v[j] / v[i]);
          j += 1;
        }
      }
      ms = max(ms, (v[i] * count));
      i += 1;
    }
  }
  var count = 0;
  {
    var j = 0;
    while ((j < v.size()))
    {
      count += (v[j] / l);
      j += 1;
    }
  }
  ms = max(ms, (l * count));
  printf("%d", ms);
  return 0;
}
