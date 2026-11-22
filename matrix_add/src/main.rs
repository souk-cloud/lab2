use std::io::{self, Write};

// 공백으로 구분된 한 줄의 숫자들을 입력받아 Vec<i32>로 반환하는 함수
fn read_numbers() -> Vec<i32> {
    let mut input = String::new();
    io::stdin().read_line(&mut input).expect("입력 읽기 실패");
    input
        .split_whitespace()
        .map(|s| s.parse::<i32>().expect("정수를 입력해야 합니다"))
        .collect()
}

fn main() {
    // 1. 행렬의 크기(행, 열) 입력받기
    print!("행렬의 행(rows)과 열(cols) 크기를 공백으로 구분해 입력하세요 (예: 2 3): ");
    io::stdout().flush().unwrap();
    let dims = read_numbers();
    if dims.len() < 2 {
        println!("행과 열 두 개의 숫자를 입력해야 합니다.");
        return;
    }
    let rows = dims[0] as usize;
    let cols = dims[1] as usize;

    // 2. 첫 번째 행렬 동적 할당(Vec<Vec<i32>>) 및 입력
    println!("\n[첫 번째 행렬 ({}x{}) 입력]", rows, cols);
    let mut matrix_a: Vec<Vec<i32>> = Vec::with_capacity(rows);
    for i in 0..rows {
        print!("{}행 원소 {}개 입력: ", i + 1, cols);
        io::stdout().flush().unwrap();
        let row = read_numbers();
        if row.len() != cols {
            println!("오류: 정확히 {}개의 숫자를 입력해야 합니다.", cols);
            return;
        }
        matrix_a.push(row);
    }

    // 3. 두 번째 행렬 동적 할당(Vec<Vec<i32>>) 및 입력
    println!("\n[두 번째 행렬 ({}x{}) 입력]", rows, cols);
    let mut matrix_b: Vec<Vec<i32>> = Vec::with_capacity(rows);
    for i in 0..rows {
        print!("{}행 원소 {}개 입력: ", i + 1, cols);
        io::stdout().flush().unwrap();
        let row = read_numbers();
        if row.len() != cols {
            println!("오류: 정확히 {}개의 숫자를 입력해야 합니다.", cols);
            return;
        }
        matrix_b.push(row);
    }

    // 4. 두 행렬 더하기 (결과도 Vec에 동적 저장)
    let mut result: Vec<Vec<i32>> = Vec::with_capacity(rows);
    for i in 0..rows {
        let mut sum_row: Vec<i32> = Vec::with_capacity(cols);
        for j in 0..cols {
            sum_row.push(matrix_a[i][j] + matrix_b[i][j]);
        }
        result.push(sum_row);
    }

    // 5. 결과 출력
    println!("\n[두 행렬의 합 결과]");
    for row in &result {
        for val in row {
            print!("{:4} ", val);
        }
        println!();
    }
}
