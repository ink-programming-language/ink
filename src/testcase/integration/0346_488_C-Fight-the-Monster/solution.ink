// Translated from solution.cpp.

var MM = 100005;

var MOD = 1000000007;

func judge(hpy: dynamic, atky: dynamic, defy: dynamic, hpm: dynamic, atkm: dynamic, defm: dynamic)
{
  var a = max(0, (atky - defm));
  var b = max(0, (atkm - defy));
  if ((a == 0))
  {
    return false;
  }
  if ((b == 0))
  {
    return true;
  }
  var c = if (((hpm % a) == 0)) (hpm / a) else ((hpm / a) + 1);
  var d = if (((hpy % b) == 0)) (hpy / b) else ((hpy / b) + 1);
  if ((d > c))
  {
    return true;
  }
  return false;
}

func main()
{
  var h: dynamic;
  var a: dynamic;
  var d: dynamic;
  var hpy: dynamic;
  var atky: dynamic;
  var defy: dynamic;
  var hpm: dynamic;
  var atkm: dynamic;
  var defm: dynamic;
  scanf("%d%d%d", (&hpy), (&atky), (&defy));
  scanf("%d%d%d", (&hpm), (&atkm), (&defm));
  scanf("%d%d%d", (&h), (&a), (&d));
  var ans = ((1 << 30));
  {
    var i = 0;
    while ((i <= 1000))
    {
      {
        var j = 0;
        while ((j <= 1000))
        {
          {
            var k = 0;
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
