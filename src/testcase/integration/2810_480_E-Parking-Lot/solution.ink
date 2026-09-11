// Translated from solution.cpp.

var inf: dynamic = 0x3f3f3f3f;

var infLL: dynamic = 0x3f3f3f3f3f3f3f3f;

var maxn: dynamic = (2000 + 5);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var g: dynamic = cpp_array(maxn, maxn);

var x: dynamic = cpp_array(maxn);

var y: dynamic = cpp_array(maxn);

var up: dynamic = cpp_array(maxn, maxn);

var down: dynamic = cpp_array(maxn, maxn);

var cur: dynamic = cpp_uninitialized();

var ret: dynamic = cpp_array(maxn);

func update(c: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < (n)))
    {
      up[i][c] = ( ((g[i][c] == cpp_char("."))) ? (( (i) ? up[(i - 1)][c] : 0) + 1) : 0);
      i += 1;
    }
  }
  {
    var i: dynamic = (n - 1);
    while ((i >= 0))
    {
      down[i][c] = ( ((g[i][c] == cpp_char("."))) ? (( ((i < (n - 1))) ? down[(i + 1)][c] : 0) + 1) : 0);
      i -= 1;
    }
  }
}

func push(deq: dynamic, v: dynamic) -> dynamic
{
  while (((!deq.empty()) && (deq.back() > v)))
  {
    deq.pop_back();
  }
  deq.push_back(v);
}

func pop(deq: dynamic, v: dynamic) -> dynamic
{
  if ((v == deq.front()))
  {
    deq.pop_front();
  }
}

func check(r: dynamic, d: dynamic) -> dynamic
{
  if (((d > n) || (d > m)))
  {
    return false;
  }
  var deq1: dynamic = cpp_uninitialized();
  var deq2: dynamic = cpp_uninitialized();
  {
    var j: dynamic = 0;
    while ((j < (m)))
    {
      push(deq1, up[r][j]);
      push(deq2, down[r][j]);
      if ((j >= (d - 1)))
      {
        if ((((deq1.front() + deq2.front()) - 1) >= d))
        {
          return true;
        }
        pop(deq1, up[r][((j - d) + 1)]);
        pop(deq2, down[r][((j - d) + 1)]);
      }
      j += 1;
    }
  }
  return false;
}

func main() -> dynamic
{
  scanf("%d%d%d", (&n), (&m), (&k));
  {
    var i: dynamic = 0;
    while ((i < (n)))
    {
      scanf("%s", g[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < (k)))
    {
      scanf("%d%d", (&x[i]), (&y[i]));
      x[i] -= 1;
      y[i] -= 1;
      g[x[i]][y[i]] = cpp_char("X");
      i += 1;
    }
  }
  {
    var j: dynamic = 0;
    while ((j < (m)))
    {
      update(j);
      j += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < (n)))
    {
      while (check(i, (cur + 1)))
      {
        cur += 1;
      }
      i += 1;
    }
  }
  {
    var i: dynamic = (k - 1);
    while ((i >= 0))
    {
      ret[i] = cur;
      g[x[i]][y[i]] = cpp_char(".");
      update(y[i]);
      while (check(x[i], (cur + 1)))
      {
        cur += 1;
      }
      i -= 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < (k)))
    {
      printf("%d\n", ret[i]);
      i += 1;
    }
  }
  return 0;
}
