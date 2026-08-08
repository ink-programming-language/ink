// Translated from solution.cpp.

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var testcases = 1;
  while (cpp_update(testcases, "--"))
  {
    var n: dynamic;
    var k: dynamic;
    read(n, k);
    var arr = cpp_array(n);
    var freq = [0];
    {
      var i = 0;
      while ((i < n))
      {
        read(arr[i]);
        freq[arr[i]] += 1;
        i += 1;
      }
    }
    var m = 0;
    var count = 0;
    var check = false;
    {
      var i = 0;
      while ((i < 102))
      {
        if ((freq[i] > k))
        {
          var x = (freq[i] / k);
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
    var c = (((m) * k) * count);
    write((c - n), "\n");
  }
}
