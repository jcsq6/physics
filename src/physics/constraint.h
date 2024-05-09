#ifndef CONSTRAINT_H
#define CONSTRAINT_H

#include <Eigen/Dense>
#include <vector>

class constraint
{
public:
	virtual float solve(const Eigen::VectorXf &q) const = 0;
	virtual ~constraint() = default;
};

class constraint_handler
{
public:
	void compute()
	{

	}
private:
	Eigen::VectorXf q; // 3n long state vector containing positions and angles
	Eigen::VectorXf f; // 3n long force vector containing force and torque
	Eigen::MatrixXf M; // 3n x 3n mass/moment matrix
	Eigen::MatrixXf M_inv; // M^-1
	Eigen::MatrixXf J; // jacobian matrix
	std::vector<constraint *> C;
	// q''=M^-1*f=M^-1(F_external + F_constraint)

	// C'=J*q'
	// C''=J'*q'+J*q''
	// C''=J'*q'+J*M^-1*(F_external + F_constraint)=0
	// J*M^-1*F_constraint=-J'*q'-J*M^-1*F_external
	// F_constraint^T*q'=0
	// J*q'=0
	// F_constraint=J^T*(lambda)

	// J*M^-1*J^T*(lambda)=-J'*q'-J*M^-1*F_external
};

#endif