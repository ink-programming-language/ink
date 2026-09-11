// Translated from solution.cpp.

var N: dynamic = 5054;

var M: dynamic = 10054;

var n: dynamic = cpp_uninitialized();

var B: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(N);

var A: dynamic = cpp_array(N);

var ans: dynamic = cpp_array(N);

var dp: dynamic = cpp_array(M, 2);

var cur: dynamic = (*dp);

var nxt: dynamic = dp[1];

var from_cpp: dynamic = cpp_array(M, N);

func down(x: dynamic, y: dynamic) -> dynamic
{
  return  ((x > y)) ? cpp_comma(cpp_assign(x, "=", y), 1) : 0;
}

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var nj: dynamic = cpp_uninitialized();
  var u: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  var used: dynamic = cpp_uninitialized();
  var s: dynamic = 0;
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
      printf("%d%c", ans[i],  ((i == n)) ? 10 : 32);
      i += 1;
    }
  }
  return 0;
}
