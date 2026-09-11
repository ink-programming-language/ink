// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  while ((cin >> n))
  {
    var factor: dynamic = cpp_uninitialized();
    {
      var i: dynamic = 1;
      while (((i * i) <= n))
      {
        if (((n % i) == 0))
        {
          factor.push_back(i);
          if ((i != (n / i)))
          {
            factor.push_back((n / i));
          }
        }
        i += 1;
      }
    }
    sort(factor.begin(), factor.end());
    var mx: dynamic = 0;
    var mm: dynamic = (1 << 60);
    {
      var i: dynamic = 0;
      while ((i < factor.size()))
      {
        var A: dynamic = factor[i];
        {
          var j: dynamic = 0;
          while ((j < factor.size()))
          {
            var B: dynamic = factor[j];
            if ((((n % ((A * B))) == 0) && binary_search(factor.begin(), factor.end(), ((n / A) / B))))
            {
              mx = max(mx, ((((A + 1)) * ((B + 2))) * ((((n / A) / B) + 2))));
              mm = min(mm, ((((A + 1)) * ((B + 2))) * ((((n / A) / B) + 2))));
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
    write((mm - n), " ", (mx - n), "\n");
  }
  return 0;
}
