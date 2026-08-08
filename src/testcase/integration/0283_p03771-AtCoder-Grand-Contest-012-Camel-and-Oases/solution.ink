// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for ((i)=1;(i)<=(n);(i)++)");
}

func repd(i: dynamic, n: dynamic)
{
  cpp_macro("for ((i)=(n);(i)>=1;(i)--)");
}

var n: dynamic;

var m: dynamic;

var i: dynamic;

var j: dynamic;

var a = cpp_array(200005);

var d = cpp_array(200005);

var lim = cpp_array(25);

var tor = cpp_array(25, 200005);

var tol = cpp_array(25, 200005);

var dppre = cpp_array((1 << 19));

var dpsuf = cpp_array((1 << 19));

func calc(x: dynamic)
{
  var i: dynamic;
  tol[(n + 1)][x] = (n + 1);
}

func main()
{
  read(n, lim[cpp_assign(m, "=", 1)]);
  rep(i, n);
  read(a[i]);
  rep(i, (n - 1))[i] = (a[(i + 1)] - a[i]);
  while ((lim[m] > 0))
  {
    m += 1;
    lim[m] = (lim[(m - 1)] / 2);
  }
  var c = 0;
  {
    i = 1;
    while ((i <= n))
    {
      c += 1;
      i = (tor[i][1] + 1);
    }
  }
  if ((c > (m + 1)))
  {
  }
  {
    i = 0;
    while ((i < ((1 << ((m - 1))))))
    {
      dppre[i] = 0;
      dpsuf[i] = (n + 1);
      {
        j = 0;
        while ((j < ((m - 1))))
        {
          if ((((i >> j)) & 1))
          {
            dppre[i] = max(dppre[i], tor[(dppre[(i ^ ((1 << j)))] + 1)][(j + 2)]);
            dpsuf[i] = min(dpsuf[i], tol[(dpsuf[(i ^ ((1 << j)))] - 1)][(j + 2)]);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      var f = 0;
      {
        j = 0;
        while ((j < ((1 << ((m - 1))))))
        {
          f |= (((dppre[j] >= (i - 1)) && (dpsuf[(((((1 << (m - 1))) - 1)) ^ j)] <= (tor[i][1] + 1))));
          j += 1;
        }
      }
      if (f)
      {
        rep(j, ((tor[i][1] - i) + 1));
        puts("Possible");
      } else
      {
        rep(j, ((tor[i][1] - i) + 1));
        puts("Impossible");
      }
      i = (tor[i][1] + 1);
    }
  }
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
    tor[i][x] = tor[(i - 1)][x];
    if ((tor[i][x] < i))
    {
      tor[i][x] = i;
      while (((tor[i][x] < n) && (d[tor[i][x]] <= lim[x])))
      {
        tor[i][x] += 1;
      }
    }
  }

func repd(argument_0: dynamic, argument_1: dynamic)
{
    tol[i][x] = tol[(i + 1)][x];
    if ((tol[i][x] > i))
    {
      tol[i][x] = i;
      while (((tol[i][x] > 1) && (d[(tol[i][x] - 1)] <= lim[x])))
      {
        tol[i][x] -= 1;
      }
    }
  }

func rep(argument_0: dynamic, argument_1: dynamic)
{
    calc(i);
  }

func rep(argument_0: dynamic, argument_1: dynamic)
{
      puts("Impossible");
      return 0;
    }
