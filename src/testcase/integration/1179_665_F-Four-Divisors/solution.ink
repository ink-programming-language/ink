// Translated from solution.cpp.

var Lower = cpp_array((1000000 + 1));

var Higher = cpp_array((1000000 + 1));

var Used = cpp_array((1000000 + 1));

func SumNat(n: dynamic)
{
  var p = n;
  var q = (n + 1);
  if ((p % 2))
  {
    q /= 2;
  } else
  {
    p /= 2;
  }
  return ((p) * (q));
}

func PrimePi(n: dynamic)
{
  var v = sqrt((n + 1e-9));
  var p: dynamic;
  var temp: dynamic;
  var q: dynamic;
  var j: dynamic;
  var end: dynamic;
  var i: dynamic;
  var d: dynamic;
  var t: dynamic;
  {
    var i = 0;
    while ((i <= 1000000))
    {
      Used[i] = 0;
      i += 1;
    }
  }
  Higher[1] = (n - 1);
  {
    p = 2;
    while ((p <= v))
    {
      Lower[p] = (p - 1);
      Higher[p] = (((n / p)) - 1);
      p += 1;
    }
  }
  {
    p = 2;
    while ((p <= v))
    {
      if ((Lower[p] == Lower[(p - 1)]))
      {
        p += 1;
        continue;
      }
      temp = Lower[(p - 1)];
      q = (p * p);
      Higher[1] -= (Higher[p] - temp);
      j = (1 + ((p & 1)));
      end = if (((v <= (n / q)))) v else (n / q);
      {
        i = (p + j);
        while ((i <= (1 + end)))
        {
          if (Used[i])
          {
            i += j;
            continue;
          }
          d = (i * p);
          if ((d <= v))
          {
            Higher[i] -= (Higher[d] - temp);
          } else
          {
            t = (n / d);
            Higher[i] -= (Lower[t] - temp);
          }
          i += j;
        }
      }
      if ((q <= v))
      {
        {
          i = q;
          while ((i <= end))
          {
            Used[i] = 1;
            i += (p * j);
          }
        }
      }
      {
        i = v;
        while ((i >= q))
        {
          t = (i / p);
          Lower[i] -= (Lower[t] - temp);
          i -= 1;
        }
      }
      p += 1;
    }
  }
  return Higher[1];
}

var prime = cpp_array((10000000 + 1));

var piii = cpp_array((10000000 + 1));

func Prim(n: dynamic)
{
  if ((n <= 10000000))
  {
    return piii[n];
  }
  return PrimePi(n);
}

func main()
{
  var n: dynamic;
  var i: dynamic;
  var j: dynamic;
  var answer: dynamic;
  {
    i = 2;
    while (((i * i) <= 10000000))
    {
      if ((!prime[i]))
      {
        {
          j = (i * i);
          while ((j <= 10000000))
          {
            prime[j] = true;
            j += i;
          }
        }
      }
      i += 1;
    }
  }
  {
    i = 2;
    while ((i <= 10000000))
    {
      piii[i] = piii[(i - 1)];
      if ((!prime[i]))
      {
        piii[i] += 1;
      }
      i += 1;
    }
  }
  read(n);
  {
    i = 1;
    while (true)
    {
      if ((((i * i) * i) > n))
      {
        break;
      }
      i += 1;
    }
  }
  answer = Prim((i - 1));
  {
    i = 2;
    while ((i < ((n / i))))
    {
      if ((!prime[i]))
      {
        answer += ((Prim((n / i)) - Prim(i)));
      }
      i += 1;
    }
  }
  write(answer);
  return 0;
}
