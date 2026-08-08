// Translated from solution.cpp.

var t = cpp_array(2222);

var p1 = cpp_array(5555);

var p2 = cpp_array(20000);

func main()
{
  var n: dynamic;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(t[i]);
      i += 1;
    }
  }
  sort(t, (t + n));
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = (i + 1);
        while ((j < n))
        {
          p1[(t[j] - t[i])] += 1;
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < 5555))
    {
      {
        var j = 0;
        while ((j < 5555))
        {
          p2[(i + j)] += (p1[i] * p1[j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var ans = 0;
  var q = 0;
  {
    var i = 1;
    while ((i < 5555))
    {
      q += p2[(i - 1)];
      ans += (p1[i] * q);
      i += 1;
    }
  }
  ans /= ((((((cpp_cast(n) * ((n - 1))) * n) * ((n - 1))) * n) * ((n - 1))) / 8);
  write(fixed, setprecision(15), ans, cpp_char("\n"));
}
