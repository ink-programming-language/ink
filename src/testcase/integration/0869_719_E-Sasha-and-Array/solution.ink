// Translated from solution.cpp.

func readInt() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var ch: dynamic = cpp_uninitialized();
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

var MOD: dynamic = 1000000007;

var t: dynamic = cpp_array(2, 2);

class Matrix
{
  var a: dynamic = cpp_array(2, 2);
  func Matrix() -> dynamic
  {
      memset(a, 0, cpp_sizeof(a));
    }
  func operator_index(i: dynamic) -> dynamic
  {
      return (*((a + i)));
    }
  func operator_index(i: dynamic) -> dynamic
  {
      return (*((a + i)));
    }
  func operator(m1: dynamic) -> dynamic
  {
      memset(t, 0, cpp_sizeof(t));
      {
        var i: dynamic = 0;
        while ((i < 2))
        {
          {
            var k: dynamic = 0;
            while ((k < 2))
            {
              {
                var j: dynamic = 0;
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

var I: dynamic = cpp_uninitialized();

var MAX_S: dynamic = 64;

var mem: dynamic = cpp_array(MAX_S);

func f(n: dynamic) -> dynamic
{
  var A: dynamic = cpp_uninitialized();
  A[0][0] = cpp_assign(A[1][1], "=", 1);
  {
    var i: dynamic = 0;
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

func init() -> dynamic
{
  mem[0][0][0] = cpp_assign(mem[0][0][1], "=", cpp_assign(mem[0][1][0], "=", 1));
  {
    var i: dynamic = 1;
    while ((i < MAX_S))
    {
      mem[i] = mem[(i - 1)];
      mem[i] *= mem[(i - 1)];
      i += 1;
    }
  }
}

var MAX_N: dynamic = (100000 + 3);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(MAX_N);

class SegmentTree
{
  var MAX_NODE: dynamic = cpp_uninitialized();
  var nodes: dynamic = cpp_array(MAX_NODE);
  func multiply(o: dynamic, c: dynamic) -> dynamic
  {
      var v: dynamic = nodes[o];
      v.tagMul *= c;
      v.matrix *= c;
    }
  func pushDown(o: dynamic) -> dynamic
  {
      multiply(((((o) * 2) + 1)), nodes[o].tagMul);
      multiply(((((o) * 2) + 2)), nodes[o].tagMul);
      nodes[o].tagMul = I;
    }
  func merge(a: dynamic, b: dynamic) -> dynamic
  {
      var c: dynamic = cpp_uninitialized();
      c[0][0] = (((a[0][0] + b[0][0])) % MOD);
      c[0][1] = (((a[0][1] + b[0][1])) % MOD);
      c[1][0] = (((a[1][0] + b[1][0])) % MOD);
      c[1][1] = (((a[1][1] + b[1][1])) % MOD);
      return c;
    }
  func build(o: dynamic, l: dynamic, r: dynamic, a: dynamic) -> dynamic
  {
      var v: dynamic = nodes[o];
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
  func init(n: dynamic, a: dynamic) -> dynamic
  {
      build(0, 0, n, a);
    }
  func query(o: dynamic, l: dynamic, r: dynamic, a: dynamic, b: dynamic) -> dynamic
  {
      if (((l >= a) && (r <= b)))
      {
        return nodes[o].matrix;
      } else
      {
        pushDown(o);
        var res: dynamic = cpp_uninitialized();
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
  func modify(o: dynamic, l: dynamic, r: dynamic, a: dynamic, b: dynamic, x: dynamic) -> dynamic
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
  func query(l: dynamic, r: dynamic) -> dynamic
  {
      return query(0, 0, n, l, r)[1][0];
    }
  func modify(l: dynamic, r: dynamic, c: dynamic) -> dynamic
  {
      modify(0, 0, n, l, r, f(c));
    }
}

var segmentTree: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  init();
  n = readInt();
  m = readInt();
  {
    var i: dynamic = 0;
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
    var type_cpp: dynamic = readInt();
    var l: dynamic = (readInt() - 1);
    var r: dynamic = readInt();
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
