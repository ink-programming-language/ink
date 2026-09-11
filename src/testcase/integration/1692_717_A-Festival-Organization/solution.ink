// Translated from solution.cpp.

class NS
{
  var MO: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  func Normalize(x: dynamic) -> dynamic
  {
      x %= MO;
      if ((x < 0))
      {
        x += MO;
      }
    }
  func first() -> dynamic
  {
      return a;
    }
  func second() -> dynamic
  {
      return b;
    }
  func NS(a: dynamic = 0, b: dynamic = 0) -> dynamic
  {
      self->a = cpp_construct(a);
      self->b = cpp_construct(b);
      Normalize(self->a);
      Normalize(self->b);
    }
  func Powd(a: dynamic, b: dynamic) -> dynamic
  {
      assert((b >= 0));
      var answer: dynamic = 1;
      {
        while ((b > 0))
        {
          if ((b & 1))
          {
            answer = ((answer * a) % MO);
          }
          a *= a;
          a %= MO;
          b >>= 1;
        }
      }
      return answer;
    }
  func Pair() -> dynamic
  {
      return NS(a, (-b));
    }
  func Inverse() -> dynamic
  {
      if (((!a) && (!b)))
      {
        write("0 has no INVERSE!!!", "\n");
        return NS();
      } else if ((b == 0))
      {
        return NS(Powd(a, (MO - 2)), 0);
      } else
      {
        var down: dynamic = ((((a * a) % MO) + MO) - ((((BASE * b) % MO) * b) % MO));
        return (((*self)).Pair() * Powd((down % MO), (MO - 2)));
      }
    }
  func operator_subtract() -> dynamic
  {
      return NS((-a), (-b));
    }
  func operator_add(rhs: dynamic) -> dynamic
  {
      return NS((a + rhs.a), (b + rhs.b));
    }
  func operator_subtract(rhs: dynamic) -> dynamic
  {
      return NS((a - rhs.a), (b - rhs.b));
    }
  func operator_multiply(rhs: dynamic) -> dynamic
  {
      return NS((((a * rhs.a) % MO) + ((((b * rhs.b) % MO) * BASE) % MO)), (((a * rhs.b) % MO) + ((b * rhs.a) % MO)));
    }
  func operator_multiply(scale: dynamic) -> dynamic
  {
      return ((*self) * NS(scale, 0));
    }
  func operator_divide(rhs: dynamic) -> dynamic
  {
      return ((*self) * rhs.Inverse());
    }
}

var N: dynamic = 222;

var Comb: dynamic = cpp_array(N, N);

var S: dynamic = cpp_array(N, N);

func Init() -> dynamic
{
  Comb[0][0] = cpp_assign(S[0][0], "=", 1);
  {
    var i: dynamic = 1;
    while ((i < N))
    {
      Comb[i][0] = cpp_assign(Comb[i][i], "=", 1);
      S[i][0] = 0;
      S[i][i] = (S[(i - 1)][(i - 1)] * NS5(i).Inverse());
      {
        var j: dynamic = 1;
        while ((j < i))
        {
          Comb[i][j] = (Comb[(i - 1)][j] + Comb[(i - 1)][(j - 1)]);
          var A: dynamic = NS5(i).Inverse();
          var B: dynamic = (A - 1);
          S[i][j] = ((S[(i - 1)][j] * B) + (S[(i - 1)][(j - 1)] * A));
          j += 1;
        }
      }
      i += 1;
    }
  }
}

func Powd(a: dynamic, b: dynamic) -> dynamic
{
  assert((b >= 0));
  var answer: dynamic = [1];
  {
    while ((b > 0))
    {
      if ((b & 1))
      {
        answer = (answer * a);
      }
      a = (a * a);
      b >>= 1;
    }
  }
  return answer;
}

func Calc(x: dynamic, start: dynamic, magic: dynamic) -> dynamic
{
  var result: dynamic = (NS5(1) - Powd(magic, x));
  result = (start * result);
  result = (result * ((NS5(1) - magic)).Inverse());
  return result;
}

func SumK(x: dynamic, b: dynamic) -> dynamic
{
  var answer: dynamic = cpp_uninitialized();
  var magic: dynamic = NS5(0, 1).Inverse();
  var P: dynamic = (NS5(1, 1) / 2);
  var Q: dynamic = P.Pair();
  {
    var i: dynamic = 0;
    while ((i <= b))
    {
      var result: dynamic = (Powd(NS5(-1), i) * Comb[b][i]);
      var start: dynamic = (Powd(P, ((b - i))) * Powd(Q, i));
      if (((start.first() == 1) && (start.second() == 0)))
      {
        answer = (answer + ((result * start) * x));
        i += 1;
        continue;
      }
      answer = (answer + (result * Calc(x, start, start)));
      i += 1;
    }
  }
  return (answer * Powd(magic, b));
}

func Solve(x: dynamic, k: dynamic) -> dynamic
{
  var answer: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i <= k))
    {
      answer = (answer + (S[k][i] * SumK(x, i)));
      i += 1;
    }
  }
  return answer.first();
}

func main() -> dynamic
{
  var k: dynamic = cpp_uninitialized();
  var l: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  Init();
  read(k, l, r);
  var answer: dynamic = (Solve((r + 2), k) - Solve((l + 1), k));
  NS5.Normalize(answer);
  write(answer, "\n");
  return 0;
}
