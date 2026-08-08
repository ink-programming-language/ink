// Translated from solution.cpp.

var n: dynamic;

var w = cpp_array(1000000);

var m = cpp_array(2000001);

func main()
{
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%d", (&w[i]));
      m[w[i]] += 1;
      i += 1;
    }
  }
  sort(w, (w + n));
  var ans = 0;
  {
    var i = 0;
    while ((i <= 1000130))
    {
      var f = m[i];
      if ((f == 0))
      {
        i += 1;
        continue;
      }
      var v1: dynamic;
      {
        var j = 0;
        while ((j < 22))
        {
          v1.push_back((f % 2));
          f /= 2;
          j += 1;
        }
      }
      var t = 1;
      {
        var j = 0;
        while ((j < 22))
        {
          m[i] -= (t * v1[j]);
          m[(j + i)] += v1[j];
          t *= 2;
          j += 1;
        }
      }
      ans += m[i];
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
