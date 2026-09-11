// Translated from solution.cpp.

var MOD: dynamic = 1000000007;

var fact: dynamic = cpp_array(200001);

var invfact: dynamic = cpp_array(200001);

var n50: dynamic = 0;

var n100: dynamic = 0;

var k: dynamic = 0;

class node
{
  var l: dynamic = cpp_array(2);
  var r: dynamic = cpp_array(2);
  var b: dynamic = cpp_uninitialized();
}

var nways: dynamic = cpp_array(2, 51, 51, 51, 51);

var shortest: dynamic = cpp_array(2, 51, 51, 51, 51);

var q: dynamic = cpp_uninitialized();

func pmod(base: dynamic, p: dynamic) -> dynamic
{
  if ((p == 0))
  {
    return 1;
  }
  var ret: dynamic = pmod(base, (p / 2));
  ret = (((ret * ret)) % MOD);
  if (((p % 2) == 1))
  {
    ret = (((ret * base)) % MOD);
  }
  return ret;
}

func invEuler(x: dynamic) -> dynamic
{
  return pmod(x, (MOD - 2));
}

func nCr(n: dynamic, r: dynamic) -> dynamic
{
  var ret: dynamic = 1;
  ret *= fact[n];
  ret = (((ret * invfact[r])) % MOD);
  ret = (((ret * invfact[(n - r)])) % MOD);
  return ret;
}

func bfs() -> dynamic
{
  memset(nways, 0, cpp_sizeof((nways)));
  memset(shortest, -1, cpp_sizeof((shortest)));
  nways[n50][n100][0][0][0] = 1;
  shortest[n50][n100][0][0][0] = 0;
  var temp: dynamic = cpp_uninitialized();
  temp.l[0] = n50;
  temp.l[1] = n100;
  temp.r[0] = 0;
  temp.r[1] = 0;
  temp.b = 0;
  q.push(temp);
  while ((!q.empty()))
  {
    temp = q.front();
    q.pop();
    var l0: dynamic = cpp_uninitialized();
    var l1: dynamic = cpp_uninitialized();
    var r0: dynamic = cpp_uninitialized();
    var r1: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
    l0 = temp.l[0];
    l1 = temp.l[1];
    r0 = temp.r[0];
    r1 = temp.r[1];
    b = temp.b;
    if ((b == 0))
    {
      {
        var i: dynamic = 0;
        while ((i <= l0))
        {
          {
            var j: dynamic = 0;
            while ((j <= l1))
            {
              if (((((50 * i) + (100 * j)) > k) || (((!i) && (!j)))))
              {
                j += 1;
                continue;
              }
              var l0: dynamic = (l0 - i);
              var l1: dynamic = (l1 - j);
              var r0: dynamic = (r0 + i);
              var r1: dynamic = (r1 + j);
              var b: dynamic = (!b);
              if (((shortest[l0][l1][r0][r1][b] == -1) || (shortest[l0][l1][r0][r1][b] > (shortest[l0][l1][r0][r1][b] + 1))))
              {
                shortest[l0][l1][r0][r1][b] = (1 + shortest[l0][l1][r0][r1][b]);
                nways[l0][l1][r0][r1][b] = (((((((nCr(l0, i) * nCr(l1, j))) % MOD)) * nways[l0][l1][r0][r1][b])) % MOD);
                temp.l[0] = l0;
                temp.l[1] = l1;
                temp.r[0] = r0;
                temp.r[1] = r1;
                temp.b = b;
                q.push(temp);
              } else if ((shortest[l0][l1][r0][r1][b] == (shortest[l0][l1][r0][r1][b] + 1)))
              {
                nways[l0][l1][r0][r1][b] = (((nways[l0][l1][r0][r1][b] + (((((((nCr(l0, i) * nCr(l1, j))) % MOD)) * nways[l0][l1][r0][r1][b])) % MOD))) % MOD);
              }
              j += 1;
            }
          }
          i += 1;
        }
      }
    } else
    {
      {
        var i: dynamic = 0;
        while ((i <= r0))
        {
          {
            var j: dynamic = 0;
            while ((j <= r1))
            {
              if (((((50 * i) + (100 * j)) > k) || (((!i) && (!j)))))
              {
                j += 1;
                continue;
              }
              var l0: dynamic = (l0 + i);
              var l1: dynamic = (l1 + j);
              var r0: dynamic = (r0 - i);
              var r1: dynamic = (r1 - j);
              var b: dynamic = (!b);
              if (((shortest[l0][l1][r0][r1][b] == -1) || (shortest[l0][l1][r0][r1][b] > (shortest[l0][l1][r0][r1][b] + 1))))
              {
                shortest[l0][l1][r0][r1][b] = (1 + shortest[l0][l1][r0][r1][b]);
                nways[l0][l1][r0][r1][b] = (((((((nCr(r0, i) * nCr(r1, j))) % MOD)) * nways[l0][l1][r0][r1][b])) % MOD);
                temp.l[0] = l0;
                temp.l[1] = l1;
                temp.r[0] = r0;
                temp.r[1] = r1;
                temp.b = b;
                q.push(temp);
              } else if ((shortest[l0][l1][r0][r1][b] == (shortest[l0][l1][r0][r1][b] + 1)))
              {
                nways[l0][l1][r0][r1][b] = (((nways[l0][l1][r0][r1][b] + (((((((nCr(r0, i) * nCr(r1, j))) % MOD)) * nways[l0][l1][r0][r1][b])) % MOD))) % MOD);
              }
              j += 1;
            }
          }
          i += 1;
        }
      }
    }
  }
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var in_cpp: dynamic = cpp_uninitialized();
  fact[0] = 1;
  fact[1] = 1;
  invfact[0] = invEuler(1);
  invfact[1] = invfact[0];
  {
    var i: dynamic = 2;
    while ((i < 200001))
    {
      fact[i] = (((fact[(i - 1)] * i)) % MOD);
      invfact[i] = invEuler(fact[i]);
      i += 1;
    }
  }
  scanf("%d %d", (&n), (&k));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%d", (&in_cpp));
      if ((in_cpp == 50))
      {
        n50 += 1;
      } else
      {
        n100 += 1;
      }
      i += 1;
    }
  }
  bfs();
  write(shortest[0][0][n50][n100][1], "\n", nways[0][0][n50][n100][1]);
}
