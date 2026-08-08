// Translated from solution.cpp.

var N = 505;

var M = 105;

var n: dynamic;

var m: dynamic;

var k: dynamic;

var top: dynamic;

var a = cpp_array(N);

var id = cpp_array(N);

var e = cpp_array(N, N);

var deg = cpp_array(N);

var d = cpp_array(N, N);

var v = cpp_array((N * N));

var prob = cpp_array(N);

var x = cpp_array((N * N));

var y = cpp_array((N * N));

var type_cpp = cpp_array((N * N));

class mat
{
  var a: dynamic = cpp_array(M, M);
  func mat()
  {
      memset(a, 0, cpp_sizeof((a)));
    }
}

var tr: dynamic;

var ans: dynamic;

func operator_multiply(a: dynamic, b: dynamic)
{
  var ans: dynamic;
  {
    var i = cpp_cast((1));
    while ((i <= cpp_cast(((*id)))))
    {
      {
        var j = cpp_cast((1));
        while ((j <= cpp_cast(((*id)))))
        {
          {
            var k = cpp_cast((1));
            while ((k <= cpp_cast(((*id)))))
            {
              ans.a[i][j] += (a.a[i][k] * b.a[k][j]);
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  return ans;
}

func gauss()
{
  {
    var i = cpp_cast((1));
    while ((i <= cpp_cast((n))))
    {
      var p = -1;
      {
        var j = cpp_cast((i));
        while ((j <= cpp_cast((n))))
        {
          if ((fabs(d[j][i]) > 1e-6))
          {
            p = j;
            break;
          }
          j += 1;
        }
      }
      if ((p != i))
      {
        {
          var k = cpp_cast((1));
          while ((k <= cpp_cast((n))))
          {
            swap(d[i][k], d[p][k]);
            k += 1;
          }
        }
        type_cpp[cpp_update(top, "++")] = 1;
        x[top] = i;
        y[top] = p;
      }
      {
        var j = cpp_cast((1));
        while ((j <= cpp_cast((n))))
        {
          if ((j != i))
          {
            var tmp = ((-d[j][i]) / d[i][i]);
            type_cpp[cpp_update(top, "++")] = 2;
            x[top] = j;
            y[top] = i;
            v[top] = tmp;
            {
              var k = cpp_cast((i));
              while ((k <= cpp_cast((n))))
              {
                d[j][k] += (tmp * d[i][k]);
                k += 1;
              }
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
}

func main()
{
  scanf("%d%d%d", (&n), (&m), (&k));
  {
    var i = cpp_cast((1));
    while ((i <= cpp_cast((n))))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  {
    var i = cpp_cast((1));
    while ((i <= cpp_cast((n))))
    {
      if (a[i])
      {
        id[i] = cpp_update((*id), "++");
      }
      i += 1;
    }
  }
  {
    var i = cpp_cast((1));
    while ((i <= cpp_cast((m))))
    {
      var x: dynamic;
      var y: dynamic;
      scanf("%d%d", (&x), (&y));
      deg[x] += 1;
      deg[y] += 1;
      e[x][y] += 1;
      e[y][x] += 1;
      i += 1;
    }
  }
  {
    var i = cpp_cast((1));
    while ((i <= cpp_cast((n))))
    {
      {
        var j = cpp_cast((1));
        while ((j <= cpp_cast((n))))
        {
          if ((!a[i]))
          {
            d[j][i] = ((1.0 * e[j][i]) / deg[j]);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = cpp_cast((1));
    while ((i <= cpp_cast((n))))
    {
      d[i][i] -= 1;
      i += 1;
    }
  }
  gauss();
  {
    var i = cpp_cast((1));
    while ((i <= cpp_cast((n))))
    {
      if (id[i])
      {
        {
          var j = cpp_cast((1));
          while ((j <= cpp_cast((n))))
          {
            prob[j] = ((-1.0 * e[j][i]) / deg[j]);
            j += 1;
          }
        }
        {
          var j = cpp_cast((1));
          while ((j <= cpp_cast((top))))
          {
            if ((type_cpp[j] == 1))
            {
              swap(prob[x[j]], prob[y[j]]);
            } else
            {
              prob[x[j]] += (v[j] * prob[y[j]]);
            }
            j += 1;
          }
        }
        {
          var j = cpp_cast((1));
          while ((j <= cpp_cast((n))))
          {
            prob[j] /= d[j][j];
            j += 1;
          }
        }
        ans.a[1][id[i]] = prob[1];
        {
          var j = cpp_cast((1));
          while ((j <= cpp_cast((n))))
          {
            if (id[j])
            {
              tr.a[id[j]][id[i]] = prob[j];
            }
            j += 1;
          }
        }
      }
      i += 1;
    }
  }
  {
    k -= 2;
    while (k)
    {
      if ((k & 1))
      {
        ans = (ans * tr);
      }
      k /= 2;
      tr = (tr * tr);
    }
  }
  printf("%.15lf\n", ans.a[1][id[n]]);
}
