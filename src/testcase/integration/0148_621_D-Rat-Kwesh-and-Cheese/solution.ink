// Translated from solution.cpp.

class Solver621D
{
}

func run()
{
  var formulas = ["x^y^z", "x^z^y", "(x^y)^z", "y^x^z", "y^z^x", "(y^x)^z", "z^x^y", "z^y^x", "(z^x)^y"];
  var functors = [__cpp_lambda_1, __cpp_lambda_2, __cpp_lambda_3, __cpp_lambda_4, __cpp_lambda_5, __cpp_lambda_6, __cpp_lambda_7, __cpp_lambda_8, __cpp_lambda_9];
  var xr: dynamic;
  var yr: dynamic;
  var zr: dynamic;
  read(xr, yr, zr);
  var x = [xr, 0.0];
  var y = [yr, 0.0];
  var z = [zr, 0.0];
  var results: dynamic;
  for (var f in functors)
  {
    results.push_back(f(x, y, z));
  }
  var compareExponentsOf = __cpp_lambda_10;
  var maxIndex = (max_element(begin(results), end(results), compareExponentsOf) - begin(results));
  write(formulas[maxIndex]);
}

func main()
{
  ios.sync_with_stdio(false);
  CurrentSolver().run();
  return 0;
}

func __cpp_lambda_1(x: dynamic, y: dynamic, z: dynamic)
{
  return (log(log(x)) + (z * log(y)));
}

func __cpp_lambda_2(x: dynamic, y: dynamic, z: dynamic)
{
  return (log(log(x)) + (y * log(z)));
}

func __cpp_lambda_3(x: dynamic, y: dynamic, z: dynamic)
{
  return ((log(log(x)) + log(y)) + log(z));
}

func __cpp_lambda_4(x: dynamic, y: dynamic, z: dynamic)
{
  return (log(log(y)) + (z * log(x)));
}

func __cpp_lambda_5(x: dynamic, y: dynamic, z: dynamic)
{
  return (log(log(y)) + (x * log(z)));
}

func __cpp_lambda_6(x: dynamic, y: dynamic, z: dynamic)
{
  return ((log(log(y)) + log(x)) + log(z));
}

func __cpp_lambda_7(x: dynamic, y: dynamic, z: dynamic)
{
  return (log(log(z)) + (y * log(x)));
}

func __cpp_lambda_8(x: dynamic, y: dynamic, z: dynamic)
{
  return (log(log(z)) + (x * log(y)));
}

func __cpp_lambda_9(x: dynamic, y: dynamic, z: dynamic)
{
  return ((log(log(z)) + log(x)) + log(y));
}

func __cpp_lambda_10(a: dynamic, b: dynamic)
{
  var aSign = if ((abs(a.imag()) > 0.001)) -1.0 else 1.0;
  var bSign = if ((abs(b.imag()) > 0.001)) -1.0 else 1.0;
  if ((aSign == bSign))
  {
    return (((a.real() * aSign) < (b.real() * bSign)));
  } else
  {
    return (aSign < bSign);
  }
}
