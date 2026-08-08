// Translated from solution.cpp.

var f = cpp_array(1000005);

var b = cpp_array(1000005);

var bit = cpp_array(1000005);

var n: dynamic;

var len: dynamic;

func update(i: dynamic, val: dynamic)
{
  while ((i <= n))
  {
    bit[i] += val;
    i += ((i & ((-i))));
  }
}

func query(i: dynamic)
{
  var sum = 0;
  while ((i > 0))
  {
    sum += bit[i];
    i -= ((i & ((-i))));
  }
  return sum;
}

func main()
{
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  var p: dynamic;
  var q: dynamic;
  var m: dynamic;
  var n: dynamic;
  var c: dynamic;
  var sm: dynamic;
  while ((cin >> n))
  {
    {
      n = n;
      f[0] = 1;
      b[0] = 0;
      i = 1;
      while ((i <= n))
      {
        update(i, 1);
        f[i] = (((f[(i - 1)] * i)) % 1000000007);
        b[i] = ((((((b[(i - 1)] * i)) % 1000000007) + ((((((((i * ((i - 1)))) / 2)) % 1000000007) * f[(i - 1)])) % 1000000007))) % 1000000007);
        i += 1;
      }
    }
    {
      sm = 0;
      p = 0;
      i = 1;
      j = (n - 1);
      while ((i <= n))
      {
        read(q);
        k = query((q - 1));
        sm = ((((sm + (k * b[j])) + ((((((k * ((k - 1))) / 2) + (p * k))) % 1000000007) * f[j]))) % 1000000007);
        p = (((p + k)) % 1000000007);
        update(q, -1);
        i += 1;
        j -= 1;
      }
    }
    write((((sm + p)) % 1000000007), "\n");
  }
  return 0;
}
