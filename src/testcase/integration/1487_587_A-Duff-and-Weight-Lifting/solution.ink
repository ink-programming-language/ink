// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var w: dynamic = cpp_array(1000000);

var m: dynamic = cpp_array(2000001);

func main() -> dynamic
{
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%d", (&w[i]));
      m[w[i]] += 1;
      i += 1;
    }
  }
  sort(w, (w + n));
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i <= 1000130))
    {
      var f: dynamic = m[i];
      if ((f == 0))
      {
        i += 1;
        continue;
      }
      var v1: dynamic = cpp_uninitialized();
      {
        var j: dynamic = 0;
        while ((j < 22))
        {
          v1.push_back((f % 2));
          f /= 2;
          j += 1;
        }
      }
      var t: dynamic = 1;
      {
        var j: dynamic = 0;
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
