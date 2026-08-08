// Translated from solution.cpp.

var cnt = cpp_array(65, 65);

var cnt2: dynamic;

var cnt5: dynamic;

var ans: dynamic;

var tmp: dynamic;

func main()
{
  var x: dynamic;
  var n: dynamic;
  read(n);
  {
    var i = 1;
    while ((i <= n))
    {
      read(x);
      tmp = llround((x * 1e9));
      cnt2 = cpp_assign(cnt5, "=", 0);
      while (((tmp % 2) == 0))
      {
        cnt2 += 1;
        tmp /= 2;
      }
      while (((tmp % 5) == 0))
      {
        cnt5 += 1;
        tmp /= 5;
      }
      {
        var j = 0;
        while ((j <= 64))
        {
          {
            var k = 0;
            while ((k <= 64))
            {
              if ((((cnt2 + j) >= 18) && ((cnt5 + k) >= 18)))
              {
                ans += cnt[j][k];
              }
              k += 1;
            }
          }
          j += 1;
        }
      }
      cnt[cnt2][cnt5] += 1;
      i += 1;
    }
  }
  write(ans);
  return 0;
}
