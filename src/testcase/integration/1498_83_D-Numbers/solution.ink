// Translated from solution.cpp.

var M = 100010;

var mod = 1000000007;

var OPT = 1000;

var f = cpp_array(M);

var g: dynamic;

var p: dynamic;

func isprime(n: dynamic)
{
  {
    var i = 2;
    while (((i * i) <= n))
    {
      if (((n % i) == 0))
      {
        return 0;
      }
      i += 1;
    }
  }
  return 1;
}

var NN: dynamic;

var DD: dynamic;

var ANS: dynamic;

func track(i: dynamic, taken: dynamic, sum: dynamic)
{
  if ((p[i] >= DD))
  {
    if (((taken % 2) == 0))
    {
      ANS += (NN / ((sum * DD)));
    } else
    {
      ANS -= (NN / ((sum * DD)));
    }
  } else
  {
    if (((sum * p[i]) <= (NN / DD)))
    {
      track((i + 1), (taken + 1), (sum * p[i]));
    }
    track((i + 1), taken, sum);
  }
}

func solve_for(n: dynamic, d: dynamic)
{
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  if ((n < d))
  {
    return 0;
  }
  if ((d < OPT))
  {
    NN = n;
    DD = d;
    ANS = 0;
    track(0, 0, 1);
    return ANS;
  } else
  {
    var r = (n / d);
    {
      i = 0;
      while ((i <= r))
      {
        g[i] = 0;
        i += 1;
      }
    }
    {
      i = 0;
      while ((p[i] < d))
      {
        {
          j = 0;
          while ((j <= r))
          {
            g[j] = 1;
            j += p[i];
          }
        }
        i += 1;
      }
    }
    var sum = 0;
    {
      i = 1;
      while ((i <= r))
      {
        sum += ((g[i] == 0));
        i += 1;
      }
    }
    return sum;
  }
  return 0;
}

func main()
{
  ios_base.sync_with_stdio(false);
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  var a: dynamic;
  var b: dynamic;
  var sum = 0;
  {
    i = 2;
    while ((i < M))
    {
      if ((!f[i]))
      {
        p.push_back(i);
        {
          j = i;
          while ((j < M))
          {
            f[j] = 1;
            j += i;
          }
        }
      }
      i += 1;
    }
  }
  scanf("%d%d%d", (&a), (&b), (&k));
  if ((isprime(k) == 0))
  {
    printf("%d\n", 0);
  } else
  {
    if (((cpp_cast(k) * cpp_cast(k)) > cpp_cast(b)))
    {
      printf("%d\n", ((k >= a) && (k <= b)));
    } else
    {
      printf("%d\n", (solve_for(b, k) - solve_for((a - 1), k)));
    }
  }
  return 0;
}
