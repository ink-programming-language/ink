// Translated from solution.cpp.

func gi(x: dynamic)
{
  var r = cpp_array((((1 << 17)) + 16));
  var s = r;
  var l = (r + ((1 << 17)));
  x = 0;
  if ((!(*s)))
  {
    memset(r, 0, ((1 << 17)));
    cin.read(r, ((1 << 17)));
    s = r;
  }
  while (((*s) && ((((*s) < 48) || ((*s) > 57)))))
  {
    s += 1;
  }
  while ((((*s) >= 48) && ((*s) <= 57)))
  {
    x = (((x * 10) + (*s)) - 48);
    s += 1;
    if ((s == l))
    {
      memset(r, 0, ((1 << 17)));
      cin.read(r, ((1 << 17)));
      s = r;
    }
  }
  s += 1;
}

var QQ = cpp_array((((1 << 17)) + 16));

var OP = QQ;

var LP = (QQ + ((1 << 17)));

func pn(x: dynamic)
{
  var B = OP;
  var c: dynamic;
  var E: dynamic;
  var t: dynamic;
  if ((!x))
  {
    t = (x / 10);
    c = ((x - (10 * t)) + 48);
    (*cpp_update(OP, "++")) = c;
    x = t;
  }
  while (x)
  {
    t = (x / 10);
    c = ((x - (10 * t)) + 48);
    (*cpp_update(OP, "++")) = c;
    x = t;
  }
  E = (OP - 1);
  while ((B < E))
  {
    swap((*B), (*E));
    B += 1;
    E -= 1;
  }
  if ((OP > LP))
  {
    cout.write(QQ, (OP - QQ));
    OP = QQ;
  }
}

func pc(c: dynamic)
{
  (*cpp_update(OP, "++")) = c;
}

var N: dynamic;

var A = cpp_array((1000006));

var g: dynamic;

var l: dynamic;

var G = cpp_array((1000006));

var L = cpp_array((1000006));

var P = cpp_array((1000006));

var a: dynamic;

var W: dynamic;

var S: dynamic;

var X: dynamic;

func main(argument_0: dynamic)
{
  ios_base.sync_with_stdio(0);
  gi(N);
  {
    var i = cpp_construct(0);
    while ((i < N))
    {
      gi(A[(i + 1)]);
      i += 1;
    }
  }
  {
    var k = cpp_construct(1);
    while ((k < (N + 1)))
    {
      if ((A[k] > k))
      {
        g += 1;
        S += cpp_assign(a, "=", (A[k] - k));
        G[a] -= 1;
        L[a] += 1;
        P[(N - k)] += (((2 * A[k]) - 1) - N);
        G[(N - k)] += 1;
        L[(N - k)] -= 1;
      } else
      {
        l += 1;
        S += (k - A[k]);
        a = ((N - k) + A[k]);
        G[a] -= 1;
        L[a] += 1;
        P[(N - k)] += (((2 * A[k]) - 1) - N);
        G[(N - k)] += 1;
        L[(N - k)] -= 1;
      }
      k += 1;
    }
  }
  X = S;
  {
    var k = cpp_construct(0);
    while ((k < N))
    {
      l += L[k];
      g += G[k];
      S += (((P[k] + l) - g) + 1);
      if ((S < X))
      {
        X = S;
        W = (k + 1);
      }
      k += 1;
    }
  }
  printf("%lld %d\n", X, W);
  return 0;
}
