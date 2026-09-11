// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var l: dynamic = cpp_uninitialized();

var v: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  scanf("%d %d", (&n), (&l));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var x: dynamic = cpp_uninitialized();
      scanf("%d", (&x));
      if ((x >= l))
      {
        v.push_back(x);
      }
      i += 1;
    }
  }
  sort(v.begin(), v.end());
  var ms: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < v.size()))
    {
      var count: dynamic = 0;
      {
        var j: dynamic = i;
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
  var count: dynamic = 0;
  {
    var j: dynamic = 0;
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
