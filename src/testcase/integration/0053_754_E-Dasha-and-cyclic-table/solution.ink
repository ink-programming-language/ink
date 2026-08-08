// Translated from solution.cpp.

var INF = 0x3f3f3f3f;

var INFL = 0x3f3f3f3f3f3f3f3f;

func amin(x: dynamic, y: dynamic)
{
  if ((y < x))
  {
    x = y;
  }
}

func amax(x: dynamic, y: dynamic)
{
  if ((x < y))
  {
    x = y;
  }
}

class FFTCoeffTable
{
  var logn: dynamic;
  var n: dynamic;
  var coeffs: dynamic = cpp_array(n);
  func FFTCoeffTable()
  {
      var PI = 3.141592653589793238462643383279;
      {
        var i = 0;
        while ((i < n))
        {
          var theta = (((2 * PI) * i) / n);
          coeffs[i] = complex(cos(theta), sin(theta));
          i += 1;
        }
      }
    }
}

var fftCoeffTable: dynamic;

func fft_core(x: dynamic, logn: dynamic, sign: dynamic)
{
  var n = (1 << logn);
  {
    var i = 1;
    var j = 0;
    while ((i < n))
    {
      var h = (n >> 1);
      while (((((cpp_assign(j, "^=", h)) & h)) == 0))
      {
        h >>= 1;
      }
      if ((i < j))
      {
        swap(x[i], x[j]);
      }
      i += 1;
    }
  }
  {
    var logm = 1;
    while ((logm <= logn))
    {
      var winc = (((1 << ((fftCoeffTable.logn - logm)))) * sign);
      if ((winc < 0))
      {
        winc += fftCoeffTable.n;
      }
      var h = (1 << ((logm - 1)));
      {
        var i = 0;
        while ((i < n))
        {
          var wk = 0;
          {
            var j = i;
            while ((j < (i + h)))
            {
              var w = fftCoeffTable.coeffs[wk];
              var k = (j + h);
              var lr = ((x[k].real() * w.real()) - (x[k].imag() * w.imag()));
              var li = ((x[k].real() * w.imag()) + (x[k].imag() * w.real()));
              x[k] = complex((x[j].real() - lr), (x[j].imag() - li));
              x[j] = complex((x[j].real() + lr), (x[j].imag() + li));
              if (((cpp_assign(wk, "+=", winc)) >= fftCoeffTable.n))
              {
                wk -= fftCoeffTable.n;
              }
              j += 1;
            }
          }
          i += (1 << logm);
        }
      }
      logm += 1;
    }
  }
}

func fft(logn: dynamic, a: dynamic)
{
  fft_core(a, logn, +1);
}

func inverse_fft(logn: dynamic, a: dynamic)
{
  fft_core(a, logn, -1);
  var inv = (double(1) / ((1 << logn)));
  {
    int_cpp(i) = 0;
    while (((i) < cpp_cast(((1 << logn)))))
    {
      a[i] *= inv;
      (i) += 1;
    }
  }
}

func FFT2D(logh: dynamic, H: dynamic, logw: dynamic, W: dynamic, A: dynamic)
{
  assert((A.size() == (1 << logh)));
  {
    int_cpp(i) = 0;
    while (((i) < cpp_cast((H))))
    {
      assert((A[i].size() == (1 << logw)));
      fft(logw, A[i].data());
      (i) += 1;
    }
  }
  var tmp = cpp_construct((1 << logh));
  {
    int_cpp(j) = 0;
    while (((j) < cpp_cast(((1 << logw)))))
    {
      {
        int_cpp(i) = 0;
        while (((i) < cpp_cast(((1 << logh)))))
        {
          tmp[i] = A[i][j];
          (i) += 1;
        }
      }
      fft(logh, tmp.data());
      {
        int_cpp(i) = 0;
        while (((i) < cpp_cast(((1 << logh)))))
        {
          A[i][j] = tmp[i];
          (i) += 1;
        }
      }
      (j) += 1;
    }
  }
}

func FFT2Dinv(logh: dynamic, H: dynamic, logw: dynamic, W: dynamic, A: dynamic)
{
  var tmp = cpp_construct((1 << logh));
  {
    int_cpp(j) = 0;
    while (((j) < cpp_cast(((1 << logw)))))
    {
      {
        int_cpp(i) = 0;
        while (((i) < cpp_cast(((1 << logh)))))
        {
          tmp[i] = A[i][j];
          (i) += 1;
        }
      }
      inverse_fft(logh, tmp.data());
      {
        int_cpp(i) = 0;
        while (((i) < cpp_cast(((1 << logh)))))
        {
          A[i][j] = tmp[i];
          (i) += 1;
        }
      }
      (j) += 1;
    }
  }
  {
    int_cpp(i) = 0;
    while (((i) < cpp_cast((H))))
    {
      inverse_fft(logw, A[i].data());
      (i) += 1;
    }
  }
}

func main()
{
  var tH: dynamic;
  var tW: dynamic;
  while ((~scanf("%d%d", (&tH), (&tW))))
  {
    {
      int_cpp(i) = 0;
      while (((i) < cpp_cast((tH))))
      {
        var buf = cpp_array(401);
        scanf("%s", buf);
        table[i] = buf;
        (i) += 1;
      }
    }
    var pH: dynamic;
    var pW: dynamic;
    scanf("%d%d", (&pH), (&pW));
    {
      int_cpp(i) = 0;
      while (((i) < cpp_cast((pH))))
      {
        var buf = cpp_array(401);
        scanf("%s", buf);
        pattern[i] = buf;
        (i) += 1;
      }
    }
    var H = (tH + pH);
    var W = (tW + pW);
    var logh = 1;
    var logw = 1;
    while (((1 << logh) < H))
    {
      logh += 1;
    }
    while (((1 << logw) < W))
    {
      logw += 1;
    }
    var matches = cpp_construct(tH, vector(tW, 0));
    {
      var a = 0;
      while ((a < 26))
      {
        var A = cpp_construct((1 << logh), vector((1 << logw)));
        var B = A;
        {
          int_cpp(i) = 0;
          while (((i) < cpp_cast(((tH + pH)))))
          {
            {
              int_cpp(j) = 0;
              while (((j) < cpp_cast(((tW + pW)))))
              {
                var c = table[(i % tH)][(j % tW)];
                var x = (c == (cpp_char("a") + a));
                var y = (c == (cpp_char("a") + ((a + 1))));
                A[(((tH + pH) - 1) - i)][(((tW + pW) - 1) - j)] = complex(x, y);
                (j) += 1;
              }
            }
            (i) += 1;
          }
        }
        {
          int_cpp(i) = 0;
          while (((i) < cpp_cast((pH))))
          {
            {
              int_cpp(j) = 0;
              while (((j) < cpp_cast((pW))))
              {
                var c = pattern[i][j];
                var x = (c == (cpp_char("a") + a));
                var y = (c == (cpp_char("a") + ((a + 1))));
                B[i][j] = complex(x, (-y));
                (j) += 1;
              }
            }
            (i) += 1;
          }
        }
        FFT2D(logh, H, logw, W, A);
        FFT2D(logh, H, logw, W, B);
        {
          int_cpp(i) = 0;
          while (((i) < cpp_cast(((1 << logh)))))
          {
            {
              int_cpp(j) = 0;
              while (((j) < cpp_cast(((1 << logw)))))
              {
                A[i][j] *= B[i][j];
                (j) += 1;
              }
            }
            (i) += 1;
          }
        }
        FFT2Dinv(logh, H, logw, W, A);
        {
          int_cpp(i) = 0;
          while (((i) < cpp_cast((tH))))
          {
            {
              int_cpp(j) = 0;
              while (((j) < cpp_cast((tW))))
              {
                var r = A[(((tH + pH) - 1) - i)][(((tW + pW) - 1) - j)];
                matches[i][j] += cpp_cast(round(r.real()));
                (j) += 1;
              }
            }
            (i) += 1;
          }
        }
        a += 2;
      }
    }
    var qs = 0;
    {
      int_cpp(i) = 0;
      while (((i) < cpp_cast((pH))))
      {
        {
          int_cpp(j) = 0;
          while (((j) < cpp_cast((pW))))
          {
            qs += (pattern[i][j] == cpp_char("?"));
            (j) += 1;
          }
        }
        (i) += 1;
      }
    }
    {
      int_cpp(i) = 0;
      while (((i) < cpp_cast((tH))))
      {
        {
          int_cpp(j) = 0;
          while (((j) < cpp_cast((tW))))
          {
            matches[i][j] += qs;
            (j) += 1;
          }
        }
        (i) += 1;
      }
    }
    var ans = cpp_construct(tH, string_cpp(tW, cpp_char("?")));
    {
      int_cpp(i) = 0;
      while (((i) < cpp_cast((tH))))
      {
        {
          int_cpp(j) = 0;
          while (((j) < cpp_cast((tW))))
          {
            ans[i][j] = if ((matches[i][j] == (pH * pW))) cpp_char("1") else cpp_char("0");
            (j) += 1;
          }
        }
        (i) += 1;
      }
    }
    {
      int_cpp(i) = 0;
      while (((i) < cpp_cast((tH))))
      {
        puts(ans[i].c_str());
        (i) += 1;
      }
    }
  }
  return 0;
}
