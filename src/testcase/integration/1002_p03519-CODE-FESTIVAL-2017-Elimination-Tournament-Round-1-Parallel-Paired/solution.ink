// Translated from solution.cpp.

var int_cpp: dynamic = dynamic;

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<(n);i++)");
}

var pb: dynamic = cpp_expression("#include<");

func all(v: dynamic) -> dynamic
{
  return cpp_expression("#include<bits/stdc++.");
}

var fi: dynamic = cpp_expression("#incl");

var se: dynamic = cpp_expression("#inclu");

func chmin(a: dynamic, b: dynamic) -> dynamic
{
  if ((a > b))
  {
    a = b;
  }
}

func chmax(a: dynamic, b: dynamic) -> dynamic
{
  if ((a < b))
  {
    a = b;
  }
}

var N: dynamic = cpp_uninitialized();

var Q: dynamic = cpp_uninitialized();

var A: dynamic = cpp_array(222222);

var B: dynamic = cpp_array(222222);

func main() -> dynamic
{
  scanf("%lld%lld", (&N), (&Q));
  N *= 2;
  rep(i, N);
  scanf("%lld", (&A[i]));
  rep(i, N);
  scanf("%lld", (&B[i]));
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var sumx: dynamic = 0;
  var base: dynamic = accumulate(A, (A + N), 0);
  {
    var i: dynamic = 1;
    while (((i + 1) < N))
    {
      if ((A[i] <= B[i]))
      {
        x.insert((B[i] - A[i]));
        sumx += (B[i] - A[i]);
      } else
      {
        y.insert((B[i] - A[i]));
      }
      i += 1;
    }
  }
  while (cpp_update(Q, "--"))
  {
    var p: dynamic = cpp_uninitialized();
    var a: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
    scanf("%lld%lld%lld", (&p), (&a), (&b));
    p -= 1;
    base -= A[p];
    if (((1 <= p) && ((p + 1) < N)))
    {
      if ((A[p] <= B[p]))
      {
        sumx -= (B[p] - A[p]);
        var it: dynamic = x.find((B[p] - A[p]));
        x.erase(it);
      } else
      {
        var it: dynamic = y.find((B[p] - A[p]));
        y.erase(it);
      }
    }
    A[p] = a;
    B[p] = b;
    base += A[p];
    if (((1 <= p) && ((p + 1) < N)))
    {
      if ((A[p] <= B[p]))
      {
        x.insert((B[p] - A[p]));
        sumx += (B[p] - A[p]);
      } else
      {
        y.insert((B[p] - A[p]));
      }
    }
    var ans: dynamic = 0;
    if (((x.size() % 2) == 0))
    {
      ans = sumx;
    } else
    {
      ans = (sumx - (*x.begin()));
      chmax(ans, (sumx + (*y.rbegin())));
    }
    printf("%lld\n", (ans + base));
  }
  return 0;
}
