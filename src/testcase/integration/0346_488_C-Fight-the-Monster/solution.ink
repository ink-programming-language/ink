// Translated from solution.cpp.

var MM: dynamic = 100005;

var MOD: dynamic = 1000000007;

func judge(hpy: dynamic, atky: dynamic, defy: dynamic, hpm: dynamic, atkm: dynamic, defm: dynamic) -> dynamic
{
  var a: dynamic = max(0, (atky - defm));
  var b: dynamic = max(0, (atkm - defy));
  if ((a == 0))
  {
    return false;
  }
  if ((b == 0))
  {
    return true;
  }
  var c: dynamic =  (((hpm % a) == 0)) ? (hpm / a) : ((hpm / a) + 1);
  var d: dynamic =  (((hpy % b) == 0)) ? (hpy / b) : ((hpy / b) + 1);
  if ((d > c))
  {
    return true;
  }
  return false;
}

func main() -> dynamic
{
  var h: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  var hpy: dynamic = cpp_uninitialized();
  var atky: dynamic = cpp_uninitialized();
  var defy: dynamic = cpp_uninitialized();
  var hpm: dynamic = cpp_uninitialized();
  var atkm: dynamic = cpp_uninitialized();
  var defm: dynamic = cpp_uninitialized();
  scanf("%d%d%d", (&hpy), (&atky), (&defy));
  scanf("%d%d%d", (&hpm), (&atkm), (&defm));
  scanf("%d%d%d", (&h), (&a), (&d));
  var ans: dynamic = ((1 << 30));
  {
    var i: dynamic = 0;
    while ((i <= 1000))
    {
      {
        var j: dynamic = 0;
        while ((j <= 1000))
        {
          {
            var k: dynamic = 0;
            while ((k <= 1000))
            {
              if (((((i * h) + (j * a)) + (k * d)) > ans))
              {
                break;
              }
              if (judge((hpy + i), (atky + j), (defy + k), hpm, atkm, defm))
              {
                ans = min(ans, (((i * h) + (j * a)) + (k * d)));
              }
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  printf("%d\n", ans);
  return 0;
}
