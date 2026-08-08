// Translated from solution.cpp.

var X = cpp_array(5010);

var A = cpp_array(5010);

var B = cpp_array(5010);

var C = cpp_array(5010);

var D = cpp_array(5010);

var f = cpp_array(5010, 5010);

func Min(x: dynamic, y: dynamic)
{
  return if ((x < y)) x else y;
}

func main()
{
  var n: dynamic;
  var sp: dynamic;
  var tp: dynamic;
  var i: dynamic;
  var j: dynamic;
  scanf("%d%d%d", (&n), (&sp), (&tp));
  {
    i = 1;
    while ((i <= n))
    {
      scanf("%d", (&X[i]));
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      scanf("%d", (&C[i]));
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      scanf("%d", (&D[i]));
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      scanf("%d", (&A[i]));
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      scanf("%d", (&B[i]));
      i += 1;
    }
  }
  if ((sp > tp))
  {
    var t = sp;
    sp = tp;
    tp = t;
    {
      i = 1;
      while ((i <= n))
      {
        t = A[i];
        A[i] = C[i];
        C[i] = t;
        t = B[i];
        B[i] = D[i];
        D[i] = t;
        i += 1;
      }
    }
  }
  memset(f, 63, cpp_sizeof((f)));
  f[0][0] = 0;
  {
    i = 1;
    while ((i < sp))
    {
      {
        j = 0;
        while ((j <= n))
        {
          if (((j > 0) && (((j > 1) || (i == 1)))))
          {
            f[i][j] = Min(f[i][j], (((f[(i - 1)][(j - 1)] - (2 * X[i])) + B[i]) + D[i]));
          }
          f[i][j] = Min(f[i][j], (((f[(i - 1)][(j + 1)] + (2 * X[i])) + A[i]) + C[i]));
          if (((j > 0) || (i == 1)))
          {
            f[i][j] = Min(f[i][j], ((f[(i - 1)][j] + A[i]) + D[i]));
          }
          if (((j > 0) || (i == 1)))
          {
            f[i][j] = Min(f[i][j], ((f[(i - 1)][j] + B[i]) + C[i]));
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
      if (((i > 1) || (sp == 1)))
      {
        f[sp][i] = Min(f[sp][i], ((f[(sp - 1)][(i - 1)] - X[sp]) + B[sp]));
      }
      f[sp][i] = Min(f[sp][i], ((f[(sp - 1)][i] + X[sp]) + A[sp]));
      i += 1;
    }
  }
  {
    i = (sp + 1);
    while ((i < tp))
    {
      {
        j = 1;
        while ((j <= n))
        {
          if ((j > 1))
          {
            f[i][j] = Min(f[i][j], (((f[(i - 1)][(j - 1)] - (2 * X[i])) + B[i]) + D[i]));
          }
          f[i][j] = Min(f[i][j], (((f[(i - 1)][(j + 1)] + (2 * X[i])) + A[i]) + C[i]));
          if ((j > 1))
          {
            f[i][j] = Min(f[i][j], ((f[(i - 1)][j] + A[i]) + D[i]));
          }
          f[i][j] = Min(f[i][j], ((f[(i - 1)][j] + B[i]) + C[i]));
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    i = 0;
    while ((i <= n))
    {
      if ((i > 0))
      {
        f[tp][i] = Min(f[tp][i], ((f[(tp - 1)][i] - X[tp]) + D[tp]));
      }
      f[tp][i] = Min(f[tp][i], ((f[(tp - 1)][(i + 1)] + X[tp]) + C[tp]));
      i += 1;
    }
  }
  {
    i = (tp + 1);
    while ((i <= n))
    {
      {
        j = 0;
        while ((j <= n))
        {
          if ((j > 1))
          {
            f[i][j] = Min(f[i][j], (((f[(i - 1)][(j - 1)] - (2 * X[i])) + B[i]) + D[i]));
          }
          f[i][j] = Min(f[i][j], (((f[(i - 1)][(j + 1)] + (2 * X[i])) + A[i]) + C[i]));
          if ((j > 0))
          {
            f[i][j] = Min(f[i][j], ((f[(i - 1)][j] + A[i]) + D[i]));
          }
          if ((j > 0))
          {
            f[i][j] = Min(f[i][j], ((f[(i - 1)][j] + B[i]) + C[i]));
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  printf("%I64d\n", f[n][0]);
  return 0;
}
