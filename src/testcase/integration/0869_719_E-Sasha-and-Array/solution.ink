// Translated from solution.cpp.

func readInt()
{
  var n: dynamic;
  var ch: dynamic;
  n = 0;
  ch = getchar();
  while ((!isdigit(ch)))
  {
    ch = getchar();
  }
  while (isdigit(ch))
  {
    n = (((n * 10) + ch) - cpp_char("0"));
    ch = getchar();
  }
  return n;
}

var MOD = 1000000007;

var t = cpp_array(2, 2);

class Matrix
{
  var a: dynamic = cpp_array(2, 2);
  func Matrix()
  {
      memset(a, 0, cpp_sizeof(a));
    }
  func operator_index(i: dynamic)
  {
      return (*((a + i)));
    }
  func operator_index(i: dynamic)
  {
      return (*((a + i)));
    }
  func operator(m1: dynamic)
  {
      memset(t, 0, cpp_sizeof(t));
      {
        var i = 0;
        while ((i < 2))
        {
          {
            var k = 0;
            while ((k < 2))
            {
              {
                var j = 0;
                while ((j < 2))
                {
                  (cpp_assign(t[i][j], "+=", ((cpp_cast(a[i][k]) * m1.a[k][j]) % MOD))) %= MOD;
                  j += 1;
                }
              }
              k += 1;
            }
          }
          i += 1;
        }
      }
      memcpy(a, t, cpp_sizeof(t));
    }
}

var I: dynamic;

var MAX_S = 64;

var mem = cpp_array(MAX_S);

func f(n: dynamic)
{
  var A: dynamic;
  A[0][0] = cpp_assign(A[1][1], "=", 1);
  {
    var i = 0;
    while ((i < MAX_S))
    {
      if ((((n >> i)) & 1))
      {
        A *= mem[i];
      }
      i += 1;
    }
  }
  return A;
}

func init()
{
  mem[0][0][0] = cpp_assign(mem[0][0][1], "=", cpp_assign(mem[0][1][0], "=", 1));
  {
    var i = 1;
    while ((i < MAX_S))
    {
      mem[i] = mem[(i - 1)];
      mem[i] *= mem[(i - 1)];
      i += 1;
    }
  }
}

var MAX_N = (100000 + 3);

var n: dynamic;

var m: dynamic;

var a = cpp_array(MAX_N);

class SegmentTree
{
  var MAX_NODE: dynamic;
  var nodes: dynamic = cpp_array(MAX_NODE);
  func multiply(o: dynamic, c: dynamic)
  {
      var v = nodes[o];
      v.tagMul *= c;
      v.matrix *= c;
    }
  func pushDown(o: dynamic)
  {
      multiply(((((o) * 2) + 1)), nodes[o].tagMul);
      multiply(((((o) * 2) + 2)), nodes[o].tagMul);
      nodes[o].tagMul = I;
    }
  func merge(a: dynamic, b: dynamic)
  {
      var c: dynamic;
      c[0][0] = (((a[0][0] + b[0][0])) % MOD);
      c[0][1] = (((a[0][1] + b[0][1])) % MOD);
      c[1][0] = (((a[1][0] + b[1][0])) % MOD);
      c[1][1] = (((a[1][1] + b[1][1])) % MOD);
      return c;
    }
  func build(o: dynamic, l: dynamic, r: dynamic, a: dynamic)
  {
      var v = nodes[o];
      v.tagMul = I;
      if (((r - l) == 1))
      {
        v.matrix = f(a[l]);
      } else
      {
        build(((((o) * 2) + 1)), l, (((((l) + (r))) >> 1)), a);
        build(((((o) * 2) + 2)), (((((l) + (r))) >> 1)), r, a);
        v.matrix = merge(nodes[((((o) * 2) + 1))].matrix, nodes[((((o) * 2) + 2))].matrix);
      }
    }
  func init(n: dynamic, a: dynamic)
  {
      build(0, 0, n, a);
    }
  func query(o: dynamic, l: dynamic, r: dynamic, a: dynamic, b: dynamic)
  {
      if (((l >= a) && (r <= b)))
      {
        return nodes[o].matrix;
      } else
      {
        pushDown(o);
        var res: dynamic;
        if (((((((l) + (r))) >> 1)) > a))
        {
          res = merge(res, query(((((o) * 2) + 1)), l, (((((l) + (r))) >> 1)), a, b));
        }
        if (((((((l) + (r))) >> 1)) < b))
        {
          res = merge(res, query(((((o) * 2) + 2)), (((((l) + (r))) >> 1)), r, a, b));
        }
        return res;
      }
    }
  func modify(o: dynamic, l: dynamic, r: dynamic, a: dynamic, b: dynamic, x: dynamic)
  {
      if (((r <= a) || (l >= b)))
      {
        return;
      }
      if (((l >= a) && (r <= b)))
      {
        multiply(o, x);
      } else
      {
        pushDown(o);
        if (((((((l) + (r))) >> 1)) > a))
        {
          modify(((((o) * 2) + 1)), l, (((((l) + (r))) >> 1)), a, b, x);
        }
        if (((((((l) + (r))) >> 1)) < b))
        {
          modify(((((o) * 2) + 2)), (((((l) + (r))) >> 1)), r, a, b, x);
        }
        nodes[o].matrix = merge(nodes[((((o) * 2) + 1))].matrix, nodes[((((o) * 2) + 2))].matrix);
      }
    }
  func query(l: dynamic, r: dynamic)
  {
      return query(0, 0, n, l, r)[1][0];
    }
  func modify(l: dynamic, r: dynamic, c: dynamic)
  {
      modify(0, 0, n, l, r, f(c));
    }
}

var segmentTree: dynamic;

func main()
{
  init();
  n = readInt();
  m = readInt();
  {
    var i = 0;
    while ((i < n))
    {
      a[i] = readInt();
      i += 1;
    }
  }
  I[0][0] = 1;
  I[1][1] = 1;
  segmentTree.init(n, a);
  while (cpp_update(m, "--"))
  {
    var type_cpp = readInt();
    var l = (readInt() - 1);
    var r = readInt();
    if ((type_cpp == 1))
    {
      segmentTree.modify(l, r, readInt());
    } else if ((type_cpp == 2))
    {
      printf("%d\n", segmentTree.query(l, r));
    } else
    {
      assert(false);
    }
  }
  return 0;
}
