// Translated from solution.cpp.

func debug(a: dynamic, b: dynamic)
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

func isprime(x: dynamic)
{
  var till = cpp_cast(sqrt((x + 0.0)));
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
    var i = 3;
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

var n: dynamic;

var p: dynamic;

var x: dynamic;

var y: dynamic;

var a = cpp_array(1000000);

func mod(foo: dynamic)
{
  foo += p;
  return (foo - ((foo / p) * p));
}

class Matrix
{
  var row: dynamic;
  var col: dynamic;
  var body: dynamic;
  func Matrix(row: dynamic, col: dynamic)
  {
      this->row = row;
      this->col = col;
      body = vector(row, vector(col, 0));
    }
  func Matrix(matrix: dynamic)
  {
      row = cpp_cast((matrix).size());
      col = cpp_cast((matrix[0]).size());
      body = matrix;
    }
  func operator_multiply(other: dynamic)
  {
      var ret = Matrix(row, other.col);
      assert((col == other.row));
      {
        var i = 0;
        while ((i < row))
        {
          {
            var j = 0;
            while ((j < other.col))
            {
              {
                var k = 0;
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

func binpow(a: dynamic, n: dynamic)
{
  assert((n >= 1));
  var ret = a;
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

func alacazam(s: dynamic, f: dynamic, l: dynamic, x: dynamic)
{
  if ((x == 0))
  {
    return s;
  }
  return ((Matrix([[s, f, l]]) * binpow(Matrix([[3, 0, 0], [-1, 1, 0], [-1, 0, 1]]), x))).body[0][0];
}

func fibazam(f1: dynamic, f2: dynamic, x: dynamic)
{
  if ((x == 0))
  {
    return f2;
  }
  return ((Matrix([[f1, f2]]) * binpow(Matrix([[0, 1], [1, 1]]), x))).body[0][1];
}

func main()
{
  scanf(("%d " + "%l" + "ld" + " " + "%l" + "ld" + " %d"), (&n), (&x), (&y), (&p));
  var sum = 0;
  {
    var i = 0;
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
