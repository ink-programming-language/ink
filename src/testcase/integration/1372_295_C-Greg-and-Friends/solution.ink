// Translated from solution.cpp.

var MOD = 1000000007;

var fact = cpp_array(200001);

var invfact = cpp_array(200001);

var n50 = 0;

var n100 = 0;

var k = 0;

class node
{
  var l: dynamic = cpp_array(2);
  var r: dynamic = cpp_array(2);
  var b: dynamic;
}

var nways = cpp_array(2, 51, 51, 51, 51);

var shortest = cpp_array(2, 51, 51, 51, 51);

var q: dynamic;

func pmod(base: dynamic, p: dynamic)
{
  if ((p == 0))
  {
    return 1;
  }
  var ret = pmod(base, (p / 2));
  ret = (((ret * ret)) % MOD);
  if (((p % 2) == 1))
  {
    ret = (((ret * base)) % MOD);
  }
  return ret;
}

func invEuler(x: dynamic)
{
  return pmod(x, (MOD - 2));
}

func nCr(n: dynamic, r: dynamic)
{
  var ret = 1;
  ret *= fact[n];
  ret = (((ret * invfact[r])) % MOD);
  ret = (((ret * invfact[(n - r)])) % MOD);
  return ret;
}

func bfs()
{
  memset(nways, 0, cpp_sizeof((nways)));
  memset(shortest, -1, cpp_sizeof((shortest)));
  nways[n50][n100][0][0][0] = 1;
  shortest[n50][n100][0][0][0] = 0;
  var temp: dynamic;
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
    var l0: dynamic;
    var l1: dynamic;
    var r0: dynamic;
    var r1: dynamic;
    var b: dynamic;
    l0 = temp.l[0];
    l1 = temp.l[1];
    r0 = temp.r[0];
    r1 = temp.r[1];
    b = temp.b;
    if ((b == 0))
    {
      {
        var i = 0;
        while ((i <= l0))
        {
          {
            var j = 0;
            while ((j <= l1))
            {
              if (((((50 * i) + (100 * j)) > k) || (((!i) && (!j)))))
              {
                j += 1;
                continue;
              }
              var l0 = (l0 - i);
              var l1 = (l1 - j);
              var r0 = (r0 + i);
              var r1 = (r1 + j);
              var b = (!b);
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
        var i = 0;
        while ((i <= r0))
        {
          {
            var j = 0;
            while ((j <= r1))
            {
              if (((((50 * i) + (100 * j)) > k) || (((!i) && (!j)))))
              {
                j += 1;
                continue;
              }
              var l0 = (l0 + i);
              var l1 = (l1 + j);
              var r0 = (r0 - i);
              var r1 = (r1 - j);
              var b = (!b);
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

func main()
{
  var n: dynamic;
  var in_cpp: dynamic;
  fact[0] = 1;
  fact[1] = 1;
  invfact[0] = invEuler(1);
  invfact[1] = invfact[0];
  {
    var i = 2;
    while ((i < 200001))
    {
      fact[i] = (((fact[(i - 1)] * i)) % MOD);
      invfact[i] = invEuler(fact[i]);
      i += 1;
    }
  }
  scanf("%d %d", (&n), (&k));
  {
    var i = 0;
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
