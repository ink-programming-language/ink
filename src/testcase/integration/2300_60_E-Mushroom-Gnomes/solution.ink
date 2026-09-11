// Translated from solution.cpp.

func debug(a: dynamic, b: dynamic) -> dynamic
{
  {
    while ((a != b))
    {
      write((*a), cpp_char(" "));
      a += 1;
    }
  }
  write("\n");
}

func isprime(x: dynamic) -> dynamic
{
  var till: dynamic = cpp_cast(sqrt((x + 0.0)));
  if ((x <= 1))
  {
    return 0;
  }
  if ((x == 2))
  {
    return 1;
  }
  if ((((x / 2) * 2) == x))
  {
    return 0;
  }
  {
    var i: dynamic = 3;
    while ((i <= till))
    {
      if ((((x / i) * i) == x))
      {
        return 0;
      }
      i += 2;
    }
  }
  return 1;
}

var n: dynamic = cpp_uninitialized();

var p: dynamic = cpp_uninitialized();

var x: dynamic = cpp_uninitialized();

var y: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(1000000);

func mod(foo: dynamic) -> dynamic
{
  foo += p;
  return (foo - ((foo / p) * p));
}

class Matrix
{
  var row: dynamic = cpp_uninitialized();
  var col: dynamic = cpp_uninitialized();
  var body: dynamic = cpp_uninitialized();
  func Matrix(row: dynamic, col: dynamic) -> dynamic
  {
      self->row = row;
      self->col = col;
      body = vector(row, vector(col, 0));
    }
  func Matrix(matrix: dynamic) -> dynamic
  {
      row = cpp_cast((matrix).size());
      col = cpp_cast((matrix[0]).size());
      body = matrix;
    }
  func operator_multiply(other: dynamic) -> dynamic
  {
      var ret: dynamic = Matrix(row, other.col);
      assert((col == other.row));
      {
        var i: dynamic = 0;
        while ((i < row))
        {
          {
            var j: dynamic = 0;
            while ((j < other.col))
            {
              {
                var k: dynamic = 0;
                while ((k < col))
                {
                  ret.body[i][j] += mod(((1 * body[i][k]) * other.body[k][j]));
                  ret.body[i][j] = mod(ret.body[i][j]);
                  k += 1;
                }
              }
              j += 1;
            }
          }
          i += 1;
        }
      }
      return ret;
    }
}

func binpow(a: dynamic, n: dynamic) -> dynamic
{
  assert((n >= 1));
  var ret: dynamic = a;
  n -= 1;
  while ((n > 0))
  {
    if ((n & 1))
    {
      ret = (ret * a);
      n -= 1;
    }
    a = (a * a);
    n >>= 1;
  }
  return ret;
}

func alacazam(s: dynamic, f: dynamic, l: dynamic, x: dynamic) -> dynamic
{
  if ((x == 0))
  {
    return s;
  }
  return ((Matrix([[s, f, l]]) * binpow(Matrix([[3, 0, 0], [-1, 1, 0], [-1, 0, 1]]), x))).body[0][0];
}

func fibazam(f1: dynamic, f2: dynamic, x: dynamic) -> dynamic
{
  if ((x == 0))
  {
    return f2;
  }
  return ((Matrix([[f1, f2]]) * binpow(Matrix([[0, 1], [1, 1]]), x))).body[0][1];
}

func main() -> dynamic
{
  scanf(("%d " + "%l" + "ld" + " " + "%l" + "ld" + " %d"), (&n), (&x), (&y), (&p));
  var sum: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      a[i] = mod(a[i]);
      sum = mod((sum + a[i]));
      i += 1;
    }
  }
  if ((n == 1))
  {
    printf("%d", sum);
    return 0;
  }
  sum = alacazam(sum, a[0], a[(n - 1)], x);
  a[(n - 1)] = fibazam(a[(n - 2)], a[(n - 1)], x);
  sum = alacazam(sum, a[0], a[(n - 1)], y);
  printf("%d", sum);
  return EXIT_SUCCESS;
}
