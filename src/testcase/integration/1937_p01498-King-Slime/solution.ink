// Translated from solution.cpp.

func rep(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=a;i<b;i++)");
}

class UF
{
  var par: dynamic;
  func UF()
  {
      par = vector(um, 0);
      rep(i, 0, um)[i] = i;
    }
  func operator_index(x: dynamic)
  {
      return if ((par[x] == x)) x else cpp_assign(par[x], "=", operator(par[x]));
    }
  func operator_call(x: dynamic, y: dynamic)
  {
      x = operator(x);
      y = operator(y);
      if ((x != y))
      {
        par[x] = y;
      }
    }
}

var N: dynamic;

var W: dynamic;

var H: dynamic;

var X = cpp_array(40101);

var Y = cpp_array(40101);

var uf: dynamic;

func main()
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
  var isWall = false;
  rep(i, 0, N);
  if (((((X[i] == 1) || (X[i] == W))) || (((Y[i] == 1) || (Y[i] == H)))))
  {
    isWall = true;
  }
  var ans = 0;
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
