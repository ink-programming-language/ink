// Translated from solution.cpp.

func main()
{
  var stack = cpp_array(50);
  var sp = 0;
  var n: dynamic;
  var i: dynamic;
  var sum: dynamic;
  while ((scanf("%d", (&stack[cpp_update(sp, "++")])) != EOF))
  {
  }
  {
    i = 0;
    while ((i < sp))
    {
      var ans = 1000000;
      n = stack[i];
      if ((n == 0))
      {
        break;
      }
      {
        var a = 0;
        while ((a <= 50))
        {
          {
            var b = 0;
            while ((b <= 50))
            {
              {
                var c = 0;
                while ((c <= 50))
                {
                  sum = ((((380 * ((5 * ((a / 5)))))) * 0.8) + (((a % 5)) * 380));
                  sum += ((((550 * ((4 * ((b / 4)))))) * 0.85) + (((b % 4)) * 550));
                  sum += ((((850 * ((3 * ((c / 3)))))) * 0.88) + (((c % 3)) * 850));
                  if (((((a * 200) + (b * 300)) + (c * 500)) == n))
                  {
                    if ((sum < ans))
                    {
                      ans = sum;
                    }
                  }
                  c += 1;
                }
              }
              b += 1;
            }
          }
          a += 1;
        }
      }
      printf("%d\n", ans);
      i += 1;
    }
  }
  return 0;
}
