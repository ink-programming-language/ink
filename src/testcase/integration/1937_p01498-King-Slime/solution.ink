// Translated from solution.cpp.

func rep(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(int i=a;i<b;i++)");
}

class UF
{
  var par: dynamic = cpp_uninitialized();
  func UF() -> dynamic
  {
      par = vector(um, 0);
      rep(i, 0, um)[i] = i;
    }
  func operator_index(x: dynamic) -> dynamic
  {
      return  ((par[x] == x)) ? x : cpp_assign(par[x], "=", operator(par[x]));
    }
  func operator_call(x: dynamic, y: dynamic) -> dynamic
  {
      x = operator(x);
      y = operator(y);
      if ((x != y))
      {
        par[x] = y;
      }
    }
}

var N: dynamic = cpp_uninitialized();

var W: dynamic = cpp_uninitialized();

var H: dynamic = cpp_uninitialized();

var X: dynamic = cpp_array(40101);

var Y: dynamic = cpp_array(40101);

var uf: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(N, W, H);
  rep(i, 0, N);
  read(X[i], Y[i]);
  rep(i, 0, N);
  rep(j, (i + 1), N);
  if (((X[i] == X[j]) || (Y[i] == Y[j])))
  {
    uf(i, j);
  }
  var isWall: dynamic = false;
  rep(i, 0, N);
  if (((((X[i] == 1) || (X[i] == W))) || (((Y[i] == 1) || (Y[i] == H)))))
  {
    isWall = true;
  }
  var ans: dynamic = 0;
  rep(i, 0, N);
  if ((uf[i] == i))
  {
    ans += 1;
  }
  if ((ans == 1))
  {
    ans = (N - 1);
  } else
  {
    if (isWall)
    {
      ans -= 1;
    }
    ans += (N - 1);
  }
  write(ans, "\n");
}
