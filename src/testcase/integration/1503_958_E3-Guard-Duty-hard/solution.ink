// Translated from solution.cpp.

func main() -> dynamic
{
  srand(time(null));
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  var n: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  read(n);
  var A: dynamic = cpp_construct((n << 1));
  var solve: dynamic = __cpp_lambda_1;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(x, y);
      A[i] = [x, y, i];
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(x, y);
      A[(i + n)] = [x, y, (i + n)];
      i += 1;
    }
  }
  solve(0, (n << 1));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      write((ans[i] + 1), cpp_char("\n"));
      i += 1;
    }
  }
}

func __cpp_lambda_2(i: dynamic, j: dynamic) -> dynamic
{
  if ((get(A[i]) < n))
  {
    ans[get(A[i])] = (get(A[j]) - n);
  } else if ((get(A[j]) < n))
  {
    ans[get(A[j])] = (get(A[i]) - n);
  } else
  {
    assert(0);
  }
}

func __cpp_lambda_3(i: dynamic) -> dynamic
{
  return  ((get(A[i]) < n)) ? 1 : -1;
}

func __cpp_lambda_4(a: dynamic, b: dynamic) -> dynamic
{
  var x1: dynamic = cpp_uninitialized();
  var x2: dynamic = cpp_uninitialized();
  var cpp_name: dynamic = cpp_uninitialized();
  var y1: dynamic = cpp_uninitialized();
  var y2: dynamic = cpp_uninitialized();
  var stx: dynamic = cpp_uninitialized();
  var sty: dynamic = cpp_uninitialized();
  tie(x1, y1, cpp_name) = a;
  tie(x2, y2, cpp_name) = b;
  tie(stx, sty, cpp_name) = A[l];
  x1 -= stx;
  x2 -= stx;
  y1 -= sty;
  y2 -= sty;
  return ((((1 * x1) * y2) - ((1 * x2) * y1)) > 0);
}

func __cpp_lambda_1(l: dynamic, r: dynamic) -> dynamic
{
  if ((l == r))
  {
    return;
  }
  var line: dynamic = __cpp_lambda_2;
  var tp: dynamic = __cpp_lambda_3;
  if (((r - l) == 2))
  {
    line(l, (l + 1));
    return;
  }
  swap((*((A.begin() + l))), (*min_element((A.begin() + l), (A.begin() + r))));
  sort(((A.begin() + l) + 1), (A.begin() + r), __cpp_lambda_4);
  var cur: dynamic = 0;
  {
    var i: dynamic = (l + 1);
    while ((i < r))
    {
      if (cpp_binary((tp(i) != tp(l)), "and", (cur == 0)))
      {
        line(i, l);
        solve((l + 1), i);
        solve((i + 1), r);
        return;
      }
      cur += tp(i);
      i += 1;
    }
  }
}
