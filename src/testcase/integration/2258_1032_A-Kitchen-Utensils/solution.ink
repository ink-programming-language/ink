// Translated from solution.cpp.

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var testcases: dynamic = 1;
  while (cpp_update(testcases, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    var k: dynamic = cpp_uninitialized();
    read(n, k);
    var arr: dynamic = cpp_array(n);
    var freq: dynamic = [0];
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        read(arr[i]);
        freq[arr[i]] += 1;
        i += 1;
      }
    }
    var m: dynamic = 0;
    var count: dynamic = 0;
    var check: dynamic = false;
    {
      var i: dynamic = 0;
      while ((i < 102))
      {
        if ((freq[i] > k))
        {
          var x: dynamic = (freq[i] / k);
          if ((x > m))
          {
            m = x;
            if (((freq[i] % k) == 0))
            {
              check = true;
            } else
            {
              check = false;
            }
          }
        }
        if ((freq[i] != 0))
        {
          count += 1;
        }
        i += 1;
      }
    }
    if (check)
    {
      m = m;
    } else
    {
      m = (m + 1);
    }
    var c: dynamic = (((m) * k) * count);
    write((c - n), "\n");
  }
}
