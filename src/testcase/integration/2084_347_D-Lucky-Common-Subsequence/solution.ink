// Translated from solution.cpp.

var N = (100 + 1);

var s1: dynamic;

var s2: dynamic;

var v: dynamic;

var dyn = cpp_array(N, N, N);

var vu = cpp_array(N, N, N);

var p = cpp_array(26, N);

func f(i: dynamic, j: dynamic, k: dynamic)
{
  if ((k == v.size()))
  {
    return (-N);
  }
  if (((i == s1.size()) || (j == s2.size())))
  {
    return 0;
  }
  if ((!vu[i][j][k]))
  {
    vu[i][j][k] = true;
    dyn[i][j][k] = max(f((i + 1), j, k), f(i, (j + 1), k));
    if ((s1[i] == s2[j]))
    {
      dyn[i][j][k] = max(dyn[i][j][k], (f((i + 1), (j + 1), p[k][(s1[i] - cpp_char("A"))]) + 1));
    }
  }
  return dyn[i][j][k];
}

func g(i: dynamic, j: dynamic, k: dynamic, s: dynamic)
{
  if (((i == s1.size()) || (j == s2.size())))
  {
    write(s, "\n");
    return;
  }
  if ((dyn[i][j][k] == dyn[(i + 1)][j][k]))
  {
    g((i + 1), j, k, s);
  } else if ((dyn[i][j][k] == dyn[i][(j + 1)][k]))
  {
    g(i, (j + 1), k, s);
  } else
  {
    g((i + 1), (j + 1), p[k][(s1[i] - cpp_char("A"))], (s + s1[i]));
  }
}

func main()
{
  read(s1, s2, v);
  {
    var i = 0;
    while ((i < v.size()))
    {
      {
        var j = cpp_char("A");
        while ((j <= cpp_char("Z")))
        {
          {
            var k = 0;
            while ((k <= i))
            {
              if (((v.substr(k, (i - k)) + j) == v.substr(0, ((i - k) + 1))))
              {
                p[i][(j - cpp_char("A"))] = ((i - k) + 1);
                break;
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
  var r = f(0, 0, 0);
  if ((r == 0))
  {
    write(0, "\n");
  } else
  {
    g(0, 0, 0, "");
  }
  return 0;
}
