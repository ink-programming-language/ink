// Translated from solution.cpp.

var N = 5054;

var M = 10054;

var n: dynamic;

var B: dynamic;

var a = cpp_array(N);

var A = cpp_array(N);

var ans = cpp_array(N);

var dp = cpp_array(M, 2);

var cur = (*dp);

var nxt = dp[1];

var from_cpp = cpp_array(M, N);

func down(x: dynamic, y: dynamic)
{
  return if ((x > y)) cpp_comma(cpp_assign(x, "=", y), 1) else 0;
}

func main()
{
  var i: dynamic;
  var j: dynamic;
  var nj: dynamic;
  var u: dynamic;
  var v: dynamic;
  var used: dynamic;
  var s = 0;
  scanf("%d%d", (&n), (&B));
  memset(nxt, 63, cpp_sizeof((*dp)));
  (*nxt) = 0;
  {
    i = 1;
    while ((i <= n))
    {
      scanf("%d", (a + i));
      A[i] = (A[(i - 1)] + a[i]);
      s += (a[i] / 10);
      swap(cur, nxt);
      memset(nxt, 63, cpp_sizeof((*dp)));
      {
        j = 0;
        while ((j <= s))
        {
          if (((cpp_assign(used, "=", (A[(i - 1)] - cur[j]))) >= 0))
          {
            {
              u = 0;
              v = a[i];
              while (((u <= v) && (u <= ((B - used) + j))))
              {
                if (down(nxt[cpp_assign(nj, "=", (j + (v / 10)))], (cur[j] + v)))
                {
                  from_cpp[i][nj] = u;
                }
                u += 1;
                v -= 1;
              }
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  j = (min_element(nxt, (nxt + ((s + 1)))) - nxt);
  printf("%d\n", nxt[j]);
  {
    i = n;
    while (i)
    {
      ans[i] = cpp_assign(u, "=", from_cpp[i][j]);
      v = (a[i] - u);
      j -= (v / 10);
      i -= 1;
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      printf("%d%c", ans[i], if ((i == n)) 10 else 32);
      i += 1;
    }
  }
  return 0;
}
